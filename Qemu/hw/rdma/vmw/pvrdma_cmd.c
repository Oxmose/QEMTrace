/*
 * QEMU paravirtual RDMA - Command channel
 *
 * Copyright (C) 2018 Oracle
 * Copyright (C) 2018 Red Hat Inc
 *
 * Authors:
 *     Yuval Shaia <yuval.shaia@oracle.com>
 *     Marcel Apfelbaum <marcel@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#include "qemu/osdep.h"
#include "qemu/error-report.h"
#include "cpu.h"
#include "hw/hw.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_ids.h"

#include "../rdma_backend.h"
#include "../rdma_rm.h"
#include "../rdma_utils.h"

#include "pvrdma.h"
#include "standard-headers/rdma/vmw_pvrdma-abi.h"

static void *pvrdma_map_to_pdir(PCIDevice *pdev, uint64_t pdir_dma,
                                uint32_t nchunks, size_t length)
{
    uint64_t *dir, *tbl;
    int tbl_idx, dir_idx, addr_idx;
    void *host_virt = NULL, *curr_page;

    if (!nchunks) {
        pr_dbg("nchunks=0\n");
        return NULL;
    }

    dir = rdma_pci_dma_map(pdev, pdir_dma, TARGET_PAGE_SIZE);
    if (!dir) {
        error_report("PVRDMA: Failed to map to page directory");
        return NULL;
    }

    tbl = rdma_pci_dma_map(pdev, dir[0], TARGET_PAGE_SIZE);
    if (!tbl) {
        error_report("PVRDMA: Failed to map to page table 0");
        goto out_unmap_dir;
    }

    curr_page = rdma_pci_dma_map(pdev, (dma_addr_t)tbl[0], TARGET_PAGE_SIZE);
    if (!curr_page) {
        error_report("PVRDMA: Failed to map the first page");
        goto out_unmap_tbl;
    }

    host_virt = mremap(curr_page, 0, length, MREMAP_MAYMOVE);
    pr_dbg("mremap %p -> %p\n", curr_page, host_virt);
    if (host_virt == MAP_FAILED) {
        host_virt = NULL;
        error_report("PVRDMA: Failed to remap memory for host_virt");
        goto out_unmap_tbl;
    }

    rdma_pci_dma_unmap(pdev, curr_page, TARGET_PAGE_SIZE);

    pr_dbg("host_virt=%p\n", host_virt);

    dir_idx = 0;
    tbl_idx = 1;
    addr_idx = 1;
    while (addr_idx < nchunks) {
        if (tbl_idx == TARGET_PAGE_SIZE / sizeof(uint64_t)) {
            tbl_idx = 0;
            dir_idx++;
            pr_dbg("Mapping to table %d\n", dir_idx);
            rdma_pci_dma_unmap(pdev, tbl, TARGET_PAGE_SIZE);
            tbl = rdma_pci_dma_map(pdev, dir[dir_idx], TARGET_PAGE_SIZE);
            if (!tbl) {
                error_report("PVRDMA: Failed to map to page table %d", dir_idx);
                goto out_unmap_host_virt;
            }
        }

        pr_dbg("guest_dma[%d]=0x%" PRIx64 "\n", addr_idx, tbl[tbl_idx]);

        curr_page = rdma_pci_dma_map(pdev, (dma_addr_t)tbl[tbl_idx],
                                     TARGET_PAGE_SIZE);
        if (!curr_page) {
            error_report("PVRDMA: Failed to map to page %d, dir %d", tbl_idx,
                         dir_idx);
            goto out_unmap_host_virt;
        }

        mremap(curr_page, 0, TARGET_PAGE_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
               host_virt + TARGET_PAGE_SIZE * addr_idx);

        rdma_pci_dma_unmap(pdev, curr_page, TARGET_PAGE_SIZE);

        addr_idx++;

        tbl_idx++;
    }

    goto out_unmap_tbl;

out_unmap_host_virt:
    munmap(host_virt, length);
    host_virt = NULL;

out_unmap_tbl:
    rdma_pci_dma_unmap(pdev, tbl, TARGET_PAGE_SIZE);

out_unmap_dir:
    rdma_pci_dma_unmap(pdev, dir, TARGET_PAGE_SIZE);

    return host_virt;
}

static int query_port(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_query_port *cmd = &req->query_port;
    struct pvrdma_cmd_query_port_resp *resp = &rsp->query_port_resp;
    struct pvrdma_port_attr attrs = {0};

    pr_dbg("port=%d\n", cmd->port_num);

    if (rdma_backend_query_port(&dev->backend_dev,
                                (struct ibv_port_attr *)&attrs)) {
        return -ENOMEM;
    }

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_QUERY_PORT_RESP;
    resp->hdr.err = 0;

    resp->attrs.state = attrs.state;
    resp->attrs.max_mtu = attrs.max_mtu;
    resp->attrs.active_mtu = attrs.active_mtu;
    resp->attrs.phys_state = attrs.phys_state;
    resp->attrs.gid_tbl_len = MIN(MAX_PORT_GIDS, attrs.gid_tbl_len);
    resp->attrs.max_msg_sz = 1024;
    resp->attrs.pkey_tbl_len = MIN(MAX_PORT_PKEYS, attrs.pkey_tbl_len);
    resp->attrs.active_width = 1;
    resp->attrs.active_speed = 1;

    return 0;
}

static int query_pkey(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_query_pkey *cmd = &req->query_pkey;
    struct pvrdma_cmd_query_pkey_resp *resp = &rsp->query_pkey_resp;

    pr_dbg("port=%d\n", cmd->port_num);
    pr_dbg("index=%d\n", cmd->index);

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_QUERY_PKEY_RESP;
    resp->hdr.err = 0;

    resp->pkey = PVRDMA_PKEY;
    pr_dbg("pkey=0x%x\n", resp->pkey);

    return 0;
}

static int create_pd(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_create_pd *cmd = &req->create_pd;
    struct pvrdma_cmd_create_pd_resp *resp = &rsp->create_pd_resp;

    pr_dbg("context=0x%x\n", cmd->ctx_handle ? cmd->ctx_handle : 0);

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_CREATE_PD_RESP;
    resp->hdr.err = rdma_rm_alloc_pd(&dev->rdma_dev_res, &dev->backend_dev,
                                     &resp->pd_handle, cmd->ctx_handle);

    pr_dbg("ret=%d\n", resp->hdr.err);
    return resp->hdr.err;
}

static int destroy_pd(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_destroy_pd *cmd = &req->destroy_pd;

    pr_dbg("pd_handle=%d\n", cmd->pd_handle);

    rdma_rm_dealloc_pd(&dev->rdma_dev_res, cmd->pd_handle);

    return 0;
}

static int create_mr(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_create_mr *cmd = &req->create_mr;
    struct pvrdma_cmd_create_mr_resp *resp = &rsp->create_mr_resp;
    PCIDevice *pci_dev = PCI_DEVICE(dev);
    void *host_virt = NULL;

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_CREATE_MR_RESP;

    pr_dbg("pd_handle=%d\n", cmd->pd_handle);
    pr_dbg("access_flags=0x%x\n", cmd->access_flags);
    pr_dbg("flags=0x%x\n", cmd->flags);

    if (!(cmd->flags & PVRDMA_MR_FLAG_DMA)) {
        host_virt = pvrdma_map_to_pdir(pci_dev, cmd->pdir_dma, cmd->nchunks,
                                       cmd->length);
        if (!host_virt) {
            pr_dbg("Failed to map to pdir\n");
            resp->hdr.err = -EINVAL;
            goto out;
        }
    }

    resp->hdr.err = rdma_rm_alloc_mr(&dev->rdma_dev_res, cmd->pd_handle,
                                     cmd->start, cmd->length, host_virt,
                                     cmd->access_flags, &resp->mr_handle,
                                     &resp->lkey, &resp->rkey);
    if (host_virt && !resp->hdr.err) {
        munmap(host_virt, cmd->length);
    }

out:
    pr_dbg("ret=%d\n", resp->hdr.err);
    return resp->hdr.err;
}

static int destroy_mr(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_destroy_mr *cmd = &req->destroy_mr;

    pr_dbg("mr_handle=%d\n", cmd->mr_handle);

    rdma_rm_dealloc_mr(&dev->rdma_dev_res, cmd->mr_handle);

    return 0;
}

static int create_cq_ring(PCIDevice *pci_dev , PvrdmaRing **ring,
                          uint64_t pdir_dma, uint32_t nchunks, uint32_t cqe)
{
    uint64_t *dir = NULL, *tbl = NULL;
    PvrdmaRing *r;
    int rc = -EINVAL;
    char ring_name[MAX_RING_NAME_SZ];

    if (!nchunks || nchunks > PVRDMA_MAX_FAST_REG_PAGES) {
        pr_dbg("invalid nchunks: %d\n", nchunks);
        return rc;
    }

    pr_dbg("pdir_dma=0x%llx\n", (long long unsigned int)pdir_dma);
    dir = rdma_pci_dma_map(pci_dev, pdir_dma, TARGET_PAGE_SIZE);
    if (!dir) {
        pr_dbg("Failed to map to CQ page directory\n");
        goto out;
    }

    tbl = rdma_pci_dma_map(pci_dev, dir[0], TARGET_PAGE_SIZE);
    if (!tbl) {
        pr_dbg("Failed to map to CQ page table\n");
        goto out;
    }

    r = g_malloc(sizeof(*r));
    *ring = r;

    r->ring_state = (struct pvrdma_ring *)
        rdma_pci_dma_map(pci_dev, tbl[0], TARGET_PAGE_SIZE);

    if (!r->ring_state) {
        pr_dbg("Failed to map to CQ ring state\n");
        goto out_free_ring;
    }

    sprintf(ring_name, "cq_ring_%" PRIx64, pdir_dma);
    rc = pvrdma_ring_init(r, ring_name, pci_dev, &r->ring_state[1],
                          cqe, sizeof(struct pvrdma_cqe),
                          /* first page is ring state */
                          (dma_addr_t *)&tbl[1], nchunks - 1);
    if (rc) {
        goto out_unmap_ring_state;
    }

    goto out;

out_unmap_ring_state:
    /* ring_state was in slot 1, not 0 so need to jump back */
    rdma_pci_dma_unmap(pci_dev, --r->ring_state, TARGET_PAGE_SIZE);

out_free_ring:
    g_free(r);

out:
    rdma_pci_dma_unmap(pci_dev, tbl, TARGET_PAGE_SIZE);
    rdma_pci_dma_unmap(pci_dev, dir, TARGET_PAGE_SIZE);

    return rc;
}

static void destroy_cq_ring(PvrdmaRing *ring)
{
    pvrdma_ring_free(ring);
    /* ring_state was in slot 1, not 0 so need to jump back */
    rdma_pci_dma_unmap(ring->dev, --ring->ring_state, TARGET_PAGE_SIZE);
    g_free(ring);
}

static int create_cq(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_create_cq *cmd = &req->create_cq;
    struct pvrdma_cmd_create_cq_resp *resp = &rsp->create_cq_resp;
    PvrdmaRing *ring = NULL;

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_CREATE_CQ_RESP;

    resp->cqe = cmd->cqe;

    resp->hdr.err = create_cq_ring(PCI_DEVICE(dev), &ring, cmd->pdir_dma,
                                   cmd->nchunks, cmd->cqe);
    if (resp->hdr.err) {
        goto out;
    }

    pr_dbg("ring=%p\n", ring);

    resp->hdr.err = rdma_rm_alloc_cq(&dev->rdma_dev_res, &dev->backend_dev,
                                     cmd->cqe, &resp->cq_handle, ring);
    if (resp->hdr.err) {
        destroy_cq_ring(ring);
    }

    resp->cqe = cmd->cqe;

out:
    pr_dbg("ret=%d\n", resp->hdr.err);
    return resp->hdr.err;
}

static int destroy_cq(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_destroy_cq *cmd = &req->destroy_cq;
    RdmaRmCQ *cq;
    PvrdmaRing *ring;

    pr_dbg("cq_handle=%d\n", cmd->cq_handle);

    cq = rdma_rm_get_cq(&dev->rdma_dev_res, cmd->cq_handle);
    if (!cq) {
        pr_dbg("Invalid CQ handle\n");
        return -EINVAL;
    }

    ring = (PvrdmaRing *)cq->opaque;
    destroy_cq_ring(ring);

    rdma_rm_dealloc_cq(&dev->rdma_dev_res, cmd->cq_handle);

    return 0;
}

static int create_qp_rings(PCIDevice *pci_dev, uint64_t pdir_dma,
                           PvrdmaRing **rings, uint32_t scqe, uint32_t smax_sge,
                           uint32_t spages, uint32_t rcqe, uint32_t rmax_sge,
                           uint32_t rpages)
{
    uint64_t *dir = NULL, *tbl = NULL;
    PvrdmaRing *sr, *rr;
    int rc = -EINVAL;
    char ring_name[MAX_RING_NAME_SZ];
    uint32_t wqe_sz;

    if (!spages || spages > PVRDMA_MAX_FAST_REG_PAGES
        || !rpages || rpages > PVRDMA_MAX_FAST_REG_PAGES) {
        pr_dbg("invalid pages: %d, %d\n", spages, rpages);
        return rc;
    }

    pr_dbg("pdir_dma=0x%llx\n", (long long unsigned int)pdir_dma);
    dir = rdma_pci_dma_map(pci_dev, pdir_dma, TARGET_PAGE_SIZE);
    if (!dir) {
        pr_dbg("Failed to map to CQ page directory\n");
        goto out;
    }

    tbl = rdma_pci_dma_map(pci_dev, dir[0], TARGET_PAGE_SIZE);
    if (!tbl) {
        pr_dbg("Failed to map to CQ page table\n");
        goto out;
    }

    sr = g_malloc(2 * sizeof(*rr));
    rr = &sr[1];
    pr_dbg("sring=%p\n", sr);
    pr_dbg("rring=%p\n", rr);

    *rings = sr;

    pr_dbg("scqe=%d\n", scqe);
    pr_dbg("smax_sge=%d\n", smax_sge);
    pr_dbg("spages=%d\n", spages);
    pr_dbg("rcqe=%d\n", rcqe);
    pr_dbg("rmax_sge=%d\n", rmax_sge);
    pr_dbg("rpages=%d\n", rpages);

    /* Create send ring */
    sr->ring_state = (struct pvrdma_ring *)
        rdma_pci_dma_map(pci_dev, tbl[0], TARGET_PAGE_SIZE);
    if (!sr->ring_state) {
        pr_dbg("Failed to map to CQ ring state\n");
        goto out_free_sr_mem;
    }

    wqe_sz = pow2ceil(sizeof(struct pvrdma_sq_wqe_hdr) +
                      sizeof(struct pvrdma_sge) * smax_sge - 1);

    sprintf(ring_name, "qp_sring_%" PRIx64, pdir_dma);
    rc = pvrdma_ring_init(sr, ring_name, pci_dev, sr->ring_state,
                          scqe, wqe_sz, (dma_addr_t *)&tbl[1], spages);
    if (rc) {
        goto out_unmap_ring_state;
    }

    /* Create recv ring */
    rr->ring_state = &sr->ring_state[1];
    wqe_sz = pow2ceil(sizeof(struct pvrdma_rq_wqe_hdr) +
                      sizeof(struct pvrdma_sge) * rmax_sge - 1);
    sprintf(ring_name, "qp_rring_%" PRIx64, pdir_dma);
    rc = pvrdma_ring_init(rr, ring_name, pci_dev, rr->ring_state,
                          rcqe, wqe_sz, (dma_addr_t *)&tbl[1 + spages], rpages);
    if (rc) {
        goto out_free_sr;
    }

    goto out;

out_free_sr:
    pvrdma_ring_free(sr);

out_unmap_ring_state:
    rdma_pci_dma_unmap(pci_dev, sr->ring_state, TARGET_PAGE_SIZE);

out_free_sr_mem:
    g_free(sr);

out:
    rdma_pci_dma_unmap(pci_dev, tbl, TARGET_PAGE_SIZE);
    rdma_pci_dma_unmap(pci_dev, dir, TARGET_PAGE_SIZE);

    return rc;
}

static void destroy_qp_rings(PvrdmaRing *ring)
{
    pr_dbg("sring=%p\n", &ring[0]);
    pvrdma_ring_free(&ring[0]);
    pr_dbg("rring=%p\n", &ring[1]);
    pvrdma_ring_free(&ring[1]);

    rdma_pci_dma_unmap(ring->dev, ring->ring_state, TARGET_PAGE_SIZE);
    g_free(ring);
}

static int create_qp(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_create_qp *cmd = &req->create_qp;
    struct pvrdma_cmd_create_qp_resp *resp = &rsp->create_qp_resp;
    PvrdmaRing *rings = NULL;

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_CREATE_QP_RESP;

    pr_dbg("total_chunks=%d\n", cmd->total_chunks);
    pr_dbg("send_chunks=%d\n", cmd->send_chunks);

    resp->hdr.err = create_qp_rings(PCI_DEVICE(dev), cmd->pdir_dma, &rings,
                                    cmd->max_send_wr, cmd->max_send_sge,
                                    cmd->send_chunks, cmd->max_recv_wr,
                                    cmd->max_recv_sge, cmd->total_chunks -
                                    cmd->send_chunks - 1);
    if (resp->hdr.err) {
        goto out;
    }

    pr_dbg("rings=%p\n", rings);

    resp->hdr.err = rdma_rm_alloc_qp(&dev->rdma_dev_res, cmd->pd_handle,
                                     cmd->qp_type, cmd->max_send_wr,
                                     cmd->max_send_sge, cmd->send_cq_handle,
                                     cmd->max_recv_wr, cmd->max_recv_sge,
                                     cmd->recv_cq_handle, rings, &resp->qpn);
    if (resp->hdr.err) {
        destroy_qp_rings(rings);
        goto out;
    }

    resp->max_send_wr = cmd->max_send_wr;
    resp->max_recv_wr = cmd->max_recv_wr;
    resp->max_send_sge = cmd->max_send_sge;
    resp->max_recv_sge = cmd->max_recv_sge;
    resp->max_inline_data = cmd->max_inline_data;

out:
    pr_dbg("ret=%d\n", resp->hdr.err);
    return resp->hdr.err;
}

static int modify_qp(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_modify_qp *cmd = &req->modify_qp;

    pr_dbg("qp_handle=%d\n", cmd->qp_handle);

    memset(rsp, 0, sizeof(*rsp));
    rsp->hdr.response = cmd->hdr.response;
    rsp->hdr.ack = PVRDMA_CMD_MODIFY_QP_RESP;

    rsp->hdr.err = rdma_rm_modify_qp(&dev->rdma_dev_res, &dev->backend_dev,
                                 cmd->qp_handle, cmd->attr_mask,
                                 (union ibv_gid *)&cmd->attrs.ah_attr.grh.dgid,
                                 cmd->attrs.dest_qp_num,
                                 (enum ibv_qp_state)cmd->attrs.qp_state,
                                 cmd->attrs.qkey, cmd->attrs.rq_psn,
                                 cmd->attrs.sq_psn);

    pr_dbg("ret=%d\n", rsp->hdr.err);
    return rsp->hdr.err;
}

static int query_qp(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_query_qp *cmd = &req->query_qp;
    struct pvrdma_cmd_query_qp_resp *resp = &rsp->query_qp_resp;
    struct ibv_qp_init_attr init_attr;

    pr_dbg("qp_handle=%d\n", cmd->qp_handle);
    pr_dbg("attr_mask=0x%x\n", cmd->attr_mask);

    memset(rsp, 0, sizeof(*rsp));
    rsp->hdr.response = cmd->hdr.response;
    rsp->hdr.ack = PVRDMA_CMD_QUERY_QP_RESP;

    rsp->hdr.err = rdma_rm_query_qp(&dev->rdma_dev_res, &dev->backend_dev,
                                    cmd->qp_handle,
                                    (struct ibv_qp_attr *)&resp->attrs,
                                    cmd->attr_mask, &init_attr);

    pr_dbg("ret=%d\n", rsp->hdr.err);
    return rsp->hdr.err;
}

static int destroy_qp(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_destroy_qp *cmd = &req->destroy_qp;
    RdmaRmQP *qp;
    PvrdmaRing *ring;

    qp = rdma_rm_get_qp(&dev->rdma_dev_res, cmd->qp_handle);
    if (!qp) {
        pr_dbg("Invalid QP handle\n");
        return -EINVAL;
    }

    rdma_rm_dealloc_qp(&dev->rdma_dev_res, cmd->qp_handle);

    ring = (PvrdmaRing *)qp->opaque;
    destroy_qp_rings(ring);

    return 0;
}

static int create_bind(PVRDMADev *dev, union pvrdma_cmd_req *req,
                       union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_create_bind *cmd = &req->create_bind;
#ifdef PVRDMA_DEBUG
    __be64 *subnet = (__be64 *)&cmd->new_gid[0];
    __be64 *if_id = (__be64 *)&cmd->new_gid[8];
#endif

    pr_dbg("index=%d\n", cmd->index);

    if (cmd->index >= MAX_PORT_GIDS) {
        return -EINVAL;
    }

    pr_dbg("gid[%d]=0x%llx,0x%llx\n", cmd->index,
           (long long unsigned int)be64_to_cpu(*subnet),
           (long long unsigned int)be64_to_cpu(*if_id));

    /* Driver forces to one port only */
    memcpy(dev->rdma_dev_res.ports[0].gid_tbl[cmd->index].raw, &cmd->new_gid,
           sizeof(cmd->new_gid));

    /* TODO: Since drivers stores node_guid at load_dsr phase then this
     * assignment is not relevant, i need to figure out a way how to
     * retrieve MAC of our netdev */
    dev->node_guid = dev->rdma_dev_res.ports[0].gid_tbl[0].global.interface_id;
    pr_dbg("dev->node_guid=0x%llx\n",
           (long long unsigned int)be64_to_cpu(dev->node_guid));

    return 0;
}

static int destroy_bind(PVRDMADev *dev, union pvrdma_cmd_req *req,
                        union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_destroy_bind *cmd = &req->destroy_bind;

    pr_dbg("index=%d\n", cmd->index);

    if (cmd->index >= MAX_PORT_GIDS) {
        return -EINVAL;
    }

    memset(dev->rdma_dev_res.ports[0].gid_tbl[cmd->index].raw, 0,
           sizeof(dev->rdma_dev_res.ports[0].gid_tbl[cmd->index].raw));

    return 0;
}

static int create_uc(PVRDMADev *dev, union pvrdma_cmd_req *req,
                     union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_create_uc *cmd = &req->create_uc;
    struct pvrdma_cmd_create_uc_resp *resp = &rsp->create_uc_resp;

    pr_dbg("pfn=%d\n", cmd->pfn);

    memset(resp, 0, sizeof(*resp));
    resp->hdr.response = cmd->hdr.response;
    resp->hdr.ack = PVRDMA_CMD_CREATE_UC_RESP;
    resp->hdr.err = rdma_rm_alloc_uc(&dev->rdma_dev_res, cmd->pfn,
                                     &resp->ctx_handle);

    pr_dbg("ret=%d\n", resp->hdr.err);

    return 0;
}

static int destroy_uc(PVRDMADev *dev, union pvrdma_cmd_req *req,
                      union pvrdma_cmd_resp *rsp)
{
    struct pvrdma_cmd_destroy_uc *cmd = &req->destroy_uc;

    pr_dbg("ctx_handle=%d\n", cmd->ctx_handle);

    rdma_rm_dealloc_uc(&dev->rdma_dev_res, cmd->ctx_handle);

    return 0;
}
struct cmd_handler {
    uint32_t cmd;
    int (*exec)(PVRDMADev *dev, union pvrdma_cmd_req *req,
            union pvrdma_cmd_resp *rsp);
};

static struct cmd_handler cmd_handlers[] = {
    {PVRDMA_CMD_QUERY_PORT, query_port},
    {PVRDMA_CMD_QUERY_PKEY, query_pkey},
    {PVRDMA_CMD_CREATE_PD, create_pd},
    {PVRDMA_CMD_DESTROY_PD, destroy_pd},
    {PVRDMA_CMD_CREATE_MR, create_mr},
    {PVRDMA_CMD_DESTROY_MR, destroy_mr},
    {PVRDMA_CMD_CREATE_CQ, create_cq},
    {PVRDMA_CMD_RESIZE_CQ, NULL},
    {PVRDMA_CMD_DESTROY_CQ, destroy_cq},
    {PVRDMA_CMD_CREATE_QP, create_qp},
    {PVRDMA_CMD_MODIFY_QP, modify_qp},
    {PVRDMA_CMD_QUERY_QP, query_qp},
    {PVRDMA_CMD_DESTROY_QP, destroy_qp},
    {PVRDMA_CMD_CREATE_UC, create_uc},
    {PVRDMA_CMD_DESTROY_UC, destroy_uc},
    {PVRDMA_CMD_CREATE_BIND, create_bind},
    {PVRDMA_CMD_DESTROY_BIND, destroy_bind},
};

int execute_command(PVRDMADev *dev)
{
    int err = 0xFFFF;
    DSRInfo *dsr_info;

    dsr_info = &dev->dsr_info;

    pr_dbg("cmd=%d\n", dsr_info->req->hdr.cmd);
    if (dsr_info->req->hdr.cmd >= sizeof(cmd_handlers) /
                      sizeof(struct cmd_handler)) {
        pr_dbg("Unsupported command\n");
        goto out;
    }

    if (!cmd_handlers[dsr_info->req->hdr.cmd].exec) {
        pr_dbg("Unsupported command (not implemented yet)\n");
        goto out;
    }

    err = cmd_handlers[dsr_info->req->hdr.cmd].exec(dev, dsr_info->req,
                            dsr_info->rsp);
out:
    set_reg_val(dev, PVRDMA_REG_ERR, err);
    post_interrupt(dev, INTR_VEC_CMD_RING);

    return (err == 0) ? 0 : -EINVAL;
}
