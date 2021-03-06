/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_DEVICE_H
#define PLATFORM_DEVICE_H

#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_dlist.h"
#include "hdf_sref.h"
#include "osal_sem.h"
#include "osal_spinlock.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct PlatformManager;

struct PlatformDevice {
    struct HdfDeviceObject *hdfDev;    /* releated to a hdf device object */
    struct IDeviceIoService *service;  /* releated to a hdf io service object */
    struct PlatformManager *manager;   /* the platform manager it belongs to */
    int32_t number;                    /* number of the device instance */
    const char *name;                  /* name of the device instance */
    struct HdfSRef ref;                /* used for reference count */
    struct DListHead node;             /* linked to the list of a manager */
    struct DListHead notifiers;        /* list of notifier nodes */
    bool ready;                        /* indicates whether initialized */
    OsalSpinlock spin;                 /* for member protection */
    struct OsalSem released;           /* for death notification */
    void *priv;                        /* private data of the device */
};

enum PlatformEventType {
    PLAT_EVENT_INIT = 0,   /* a platform device already initialized */
    PLAT_EVENT_DEAD = 1,   /* a platform device going to die */
    PLAT_EVENT_MAX  = 2,
};

struct PlatformNotifier {
    void *data;             /* private data u can take */
    /**
     * @brief the event handle funciton of this notifier.
     *
     * make sure it can be called in irq context.
     *
     * @param device Indicates the pointer to the platform device.
     * @param event Indicates the event type of this handling process.
     *
     * @since 1.0
     */
    void (*handle)(struct PlatformDevice *device, enum PlatformEventType event, void *data);
};

struct PlatformNotifierNode {
    struct DListHead node;              /* link to notfifier node list */
    struct PlatformNotifier *notifier;  /* pointer to the notifier instance */
};

/**
 * @brief Initialize a platform device.
 *
 * Initialize members of a platform device.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
void PlatformDeviceInit(struct PlatformDevice *device);

/**
 * @brief Uninitialize a platform device.
 *
 * Uninitialize members of a platform device.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
void PlatformDeviceUninit(struct PlatformDevice *device);

/**
 * @brief Increase reference count for a platform device.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @return Returns the pointer to the paltform device on success; returns NULL otherwise.
 * @since 1.0
 */
struct PlatformDevice *PlatformDeviceGet(struct PlatformDevice *device);

/**
 * @brief Decrease reference count for a platform device.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
void PlatformDevicePut(struct PlatformDevice *device);

/**
 * @brief Register a notifier to a platform device.
 *
 * Subscribe the events of a platform device by registering a notfier.
 *
 * @param device Indicates the pointer to the platform device.
 * @param notifier Indicates the pointer to the platform notifier.
 *
 * @return Returns 0 if the notifier registered successfully; returns a negative value otherwise.
 * @since 1.0
 */
int32_t PlatformDeviceRegNotifier(struct PlatformDevice *device, struct PlatformNotifier *notifier);

/**
 * @brief Notify a platform event.
 *
 * @param device Indicates the pointer to the platform device.
 * @param event The platform event to notify.
 *
 * @return Returns 0 if the notify successfully; returns a negative value otherwise.
 * @since 1.0
 */
int32_t PlatformDeviceNotify(struct PlatformDevice *device, int32_t event);

/**
 * @brief Unregister a notifier to a platform device.
 *
 * Unsubscribe the events of a platform device by unregistering the notfier.
 *
 * @param device Indicates the pointer to the platform device.
 * @param notifier Indicates the pointer to the platform notifier.
 *
 * @since 1.0
 */
void PlatformDeviceUnregNotifier(struct PlatformDevice *device, struct PlatformNotifier *notifier);

/**
 * @brief Unregister all notifiers to a platform device.
 *
 * Unsubscribe all the events of a platform device by clearing the notfiers.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
void PlatformDeviceClearNotifier(struct PlatformDevice *device);

/**
 * @brief Add a platform device by module type.
 *
 * do not call in irq context cause can sleep
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @return Returns 0 if add successfully; returns a negative value otherwise.
 * @since 1.0
 */
int32_t PlatformDeviceAdd(struct PlatformDevice *device);

/**
 * @brief Remove a platform device by module type.
 *
 * do not call in irq context cause can sleep
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
void PlatformDeviceDel(struct PlatformDevice *device);

/**
 * @brief Get device resource node of the device.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
struct DeviceResourceNode *PlatformDeviceGetDrs(struct PlatformDevice *device);

/**
 * @brief Create a hdf device service for the platform device.
 *
 * @param device Indicates the pointer to the platform device.
 * @param dispatch Dispatch function for the service.
 *
 * @since 1.0
 */
int32_t PlatformDeviceCreateService(struct PlatformDevice *device,
    int32_t (*dispatch)(struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply));

/**
 * @brief Destroy the hdf device service for the platform device.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
int32_t PlatformDeviceDestroyService(struct PlatformDevice *device);

/**
 * @brief Bind to a hdf device object.
 *
 * @param device Indicates the pointer to the platform device.
 * @param hdfDevice Indicates the pointer to the hdf device object.
 *
 * @since 1.0
 */
int32_t PlatformDeviceBind(struct PlatformDevice *device, struct HdfDeviceObject *hdfDevice);

/**
 * @brief Unbind from a hdf device object.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
int32_t PlatformDeviceUnbind(struct PlatformDevice *device);


/**
 * @brief Get the platform device from a hdf device object.
 *
 * @param device Indicates the pointer to the platform device.
 *
 * @since 1.0
 */
struct PlatformDevice *PlatformDeviceFromHdf(struct HdfDeviceObject *device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* PLATFORM_DEVICE_H */
