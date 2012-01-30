/*
 * PROJECT:     ReactOS Universal Serial Bus Bulk Enhanced Host Controller Interface
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        drivers/usb/usbccgp/pdo.c
 * PURPOSE:     USB  device driver.
 * PROGRAMMERS:
 *              Michael Martin (michael.martin@reactos.org)
 *              Johannes Anderwald (johannes.anderwald@reactos.org)
 *              Cameron Gutman
 */

#include "usbccgp.h"

NTSTATUS
USBCCGP_PdoHandleQueryDeviceText(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp)
{
    LPWSTR Buffer;
    PPDO_DEVICE_EXTENSION PDODeviceExtension;
    LPWSTR GenericString = L"Composite USB Device";
    //
    // get device extension
    //
    PDODeviceExtension = (PPDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    //
    // is there a device description
    //
    if (PDODeviceExtension->FunctionDescriptor->FunctionDescription.Length)
    {
        //
        // allocate buffer
        //
        Buffer = AllocateItem(NonPagedPool, PDODeviceExtension->FunctionDescriptor->FunctionDescription.Length + sizeof(WCHAR));
        if (!Buffer)
        {
            //
            // no memory
            //
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // copy buffer
        //
        Irp->IoStatus.Information = (ULONG_PTR)Buffer;
        RtlCopyMemory(Buffer, PDODeviceExtension->FunctionDescriptor->FunctionDescription.Buffer, PDODeviceExtension->FunctionDescriptor->FunctionDescription.Length);
        return STATUS_SUCCESS;
    }

    //
    // FIXME use GenericCompositeUSBDeviceString
    //
    UNIMPLEMENTED
    Buffer = AllocateItem(PagedPool, (wcslen(GenericString) + 1) * sizeof(WCHAR));
    if (!Buffer)
    {
        //
        // no memory
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(Buffer, GenericString, (wcslen(GenericString) + 1) * sizeof(WCHAR));
    Irp->IoStatus.Information = (ULONG_PTR)Buffer;

    return STATUS_SUCCESS;
}

NTSTATUS
USBCCGP_PdoHandleDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp)
{
    PDEVICE_RELATIONS DeviceRelations;
    PIO_STACK_LOCATION IoStack;

    DPRINT1("USBCCGP_PdoHandleDeviceRelations\n");

    //
    // get current irp stack location
    //
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // check if relation type is BusRelations
    //
    if (IoStack->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation)
    {
        //
        // PDO handles only target device relation
        //
        return Irp->IoStatus.Status;
    }

    //
    // allocate device relations
    //
    DeviceRelations = (PDEVICE_RELATIONS)AllocateItem(PagedPool, sizeof(DEVICE_RELATIONS));
    if (!DeviceRelations)
    {
        //
        // no memory
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // initialize device relations
    //
    DeviceRelations->Count = 1;
    DeviceRelations->Objects[0] = DeviceObject;
    ObReferenceObject(DeviceObject);

    //
    // store result
    //
    Irp->IoStatus.Information = (ULONG_PTR)DeviceRelations;

    //
    // completed successfully
    //
    return STATUS_SUCCESS;
}

NTSTATUS
USBCCGP_PdoHandleQueryId(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    PUNICODE_STRING DeviceString = NULL;
    UNICODE_STRING TempString;
    PPDO_DEVICE_EXTENSION PDODeviceExtension;
    NTSTATUS Status;
    LPWSTR Buffer;

    //
    // get current irp stack location
    //
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // get device extension
    //
    PDODeviceExtension = (PPDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;


    if (IoStack->Parameters.QueryId.IdType == BusQueryDeviceID)
    {
        //
        // handle query device id
        //
        Status = USBCCGP_SyncForwardIrp(PDODeviceExtension->NextDeviceObject, Irp);

        //
        // FIXME append interface id
        //
        UNIMPLEMENTED
        return Status;
    }
    else if (IoStack->Parameters.QueryId.IdType == BusQueryHardwareIDs)
    {
        //
        // handle instance id
        //
        DeviceString = &PDODeviceExtension->FunctionDescriptor->HardwareId;
    }
    else if (IoStack->Parameters.QueryId.IdType == BusQueryInstanceID)
    {
        //
        // handle instance id
        //
        RtlInitUnicodeString(&TempString, L"0000");
        DeviceString = &TempString;
    }
    else if (IoStack->Parameters.QueryId.IdType == BusQueryCompatibleIDs)
    {
        //
        // handle instance id
        //
        DeviceString = &PDODeviceExtension->FunctionDescriptor->CompatibleId;
    }

    //
    // sanity check
    //
    ASSERT(DeviceString != NULL);

    //
    // allocate buffer
    //
    Buffer = AllocateItem(NonPagedPool, DeviceString->Length + sizeof(WCHAR));
    if (!Buffer)
    {
        //
        // no memory
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // copy buffer
    //
    Irp->IoStatus.Information = (ULONG_PTR)Buffer;
    RtlCopyMemory(Buffer, DeviceString->Buffer, DeviceString->Length);
    return STATUS_SUCCESS;
}

NTSTATUS
PDO_HandlePnp(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    PPDO_DEVICE_EXTENSION PDODeviceExtension;
    NTSTATUS Status;

    //
    // get current stack location
    //
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // get device extension
    //
    PDODeviceExtension = (PPDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    //
    // sanity check
    //
    ASSERT(PDODeviceExtension->Common.IsFDO == FALSE);

    switch(IoStack->MinorFunction)
    {
       case IRP_MN_QUERY_DEVICE_RELATIONS:
       {
           //
           // handle device relations
           //
           Status = USBCCGP_PdoHandleDeviceRelations(DeviceObject, Irp);
           break;
       }
       case IRP_MN_QUERY_DEVICE_TEXT:
       {
           //
           // handle query device text
           //
           Status = USBCCGP_PdoHandleQueryDeviceText(DeviceObject, Irp);
           break;
       }
       case IRP_MN_QUERY_ID:
       {
           //
           // handle request
           //
           Status = USBCCGP_PdoHandleQueryId(DeviceObject, Irp);
           break;
       }
       case IRP_MN_REMOVE_DEVICE:
       {
           DPRINT1("IRP_MN_REMOVE_DEVICE\n");

           /* Complete the IRP */
           Irp->IoStatus.Status = STATUS_SUCCESS;
           IoCompleteRequest(Irp, IO_NO_INCREMENT);

           /* Delete the device object */
           IoDeleteDevice(DeviceObject);

           return STATUS_SUCCESS;
       }
       case IRP_MN_QUERY_CAPABILITIES:
       {
           //
           // copy device capabilities
           //
           RtlCopyMemory(IoStack->Parameters.DeviceCapabilities.Capabilities, &PDODeviceExtension->Capabilities, sizeof(DEVICE_CAPABILITIES));

           /* Complete the IRP */
           Irp->IoStatus.Status = STATUS_SUCCESS;
           IoCompleteRequest(Irp, IO_NO_INCREMENT);
           return STATUS_SUCCESS;
       }
       case IRP_MN_START_DEVICE:
       {
           //
           // no-op for PDO
           //
           DPRINT1("[USBCCGP] PDO IRP_MN_START\n");
           Status = STATUS_SUCCESS;
           break;
       }
       default:
        {
            //
            // do nothing
            //
            Status = Irp->IoStatus.Status;
        }
    }

    //
    // complete request
    //
    if (Status != STATUS_PENDING)
    {
        //
        // store result
        //
        Irp->IoStatus.Status = Status;

        //
        // complete request
        //
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    //
    // done processing
    //
    return Status;

}

NTSTATUS
USBCCGP_BuildConfigurationDescriptor(
    PDEVICE_OBJECT DeviceObject, 
    PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    PPDO_DEVICE_EXTENSION PDODeviceExtension;
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor;
    PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
    ULONG TotalSize, Index;
    PURB Urb;
    PVOID Buffer;
    PUCHAR BufferPtr;

    //
    // get current stack location
    //
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    DPRINT1("USBCCGP_BuildConfigurationDescriptor\n");

    //
    // get device extension
    //
    PDODeviceExtension = (PPDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    //
    // get configuration descriptor
    //
    ConfigurationDescriptor = PDODeviceExtension->ConfigurationDescriptor;

    //
    // calculate size of configuration descriptor
    //
    TotalSize = sizeof(USB_CONFIGURATION_DESCRIPTOR);

    for(Index = 0; Index < PDODeviceExtension->FunctionDescriptor->NumberOfInterfaces; Index++)
    {
        //
        // get current interface descriptor
        //
        InterfaceDescriptor = PDODeviceExtension->FunctionDescriptor->InterfaceDescriptorList[Index];

        //
        // add to size and move to next descriptor
        //
        TotalSize += InterfaceDescriptor->bLength;
        InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)((ULONG_PTR)InterfaceDescriptor + InterfaceDescriptor->bLength);

        do
        {
            if ((ULONG_PTR)InterfaceDescriptor >= ((ULONG_PTR)ConfigurationDescriptor + ConfigurationDescriptor->wTotalLength))
            {
                //
                // reached end of configuration descriptor
                //
                break;
            }

            //
            // association descriptors are removed
            //
            if (InterfaceDescriptor->bDescriptorType != USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE)
            {
                if (InterfaceDescriptor->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
                {
                    //
                    // reached next descriptor
                    //
                    break;
                }

                //
                // append size
                //
                TotalSize += InterfaceDescriptor->bLength;
            }

            //
            // move to next descriptor
            //
            InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)((ULONG_PTR)InterfaceDescriptor + InterfaceDescriptor->bLength);
        }while(TRUE);
    }

    //
    // now allocate temporary buffer for the configuration descriptor
    //
    Buffer = AllocateItem(NonPagedPool, TotalSize);
    if (!Buffer)
    {
        //
        // failed to allocate buffer
        //
        DPRINT1("[USBCCGP] Failed to allocate %lu Bytes\n", TotalSize);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // first copy the configuration descriptor
    //
    RtlCopyMemory(Buffer, ConfigurationDescriptor, sizeof(USB_CONFIGURATION_DESCRIPTOR));
    BufferPtr = (PUCHAR)((ULONG_PTR)Buffer + ConfigurationDescriptor->bLength);

    for(Index = 0; Index < PDODeviceExtension->FunctionDescriptor->NumberOfInterfaces; Index++)
    {
        //
        // get current interface descriptor
        //
        InterfaceDescriptor = PDODeviceExtension->FunctionDescriptor->InterfaceDescriptorList[Index];

        //
        // copy descriptor and move to next descriptor
        //
        RtlCopyMemory(BufferPtr, InterfaceDescriptor, InterfaceDescriptor->bLength);
        BufferPtr += InterfaceDescriptor->bLength;
        InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)((ULONG_PTR)InterfaceDescriptor + InterfaceDescriptor->bLength);

        do
        {
            if ((ULONG_PTR)InterfaceDescriptor >= ((ULONG_PTR)ConfigurationDescriptor + ConfigurationDescriptor->wTotalLength))
            {
                //
                // reached end of configuration descriptor
                //
                break;
            }

            //
            // association descriptors are removed
            //
            if (InterfaceDescriptor->bDescriptorType != USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE)
            {
                if (InterfaceDescriptor->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
                {
                    //
                    // reached next descriptor
                    //
                    break;
                }

                //
                // copy descriptor
                //
                RtlCopyMemory(BufferPtr, InterfaceDescriptor, InterfaceDescriptor->bLength);
                BufferPtr += InterfaceDescriptor->bLength;
            }

            //
            // move to next descriptor
            //
            InterfaceDescriptor = (PUSB_INTERFACE_DESCRIPTOR)((ULONG_PTR)InterfaceDescriptor + InterfaceDescriptor->bLength);
        }while(TRUE);
    }

    //
    // modify configuration descriptor
    //
    ConfigurationDescriptor = Buffer;
    ConfigurationDescriptor->wTotalLength = TotalSize;
    ConfigurationDescriptor->bNumInterfaces = PDODeviceExtension->FunctionDescriptor->NumberOfInterfaces;

    //
    // get urb
    //
    Urb = (PURB)IoStack->Parameters.Others.Argument1;
    ASSERT(Urb);

    //
    // copy descriptor
    //
    RtlCopyMemory(Urb->UrbControlDescriptorRequest.TransferBuffer, Buffer, min(TotalSize, Urb->UrbControlDescriptorRequest.TransferBufferLength));

    //
    // store final size
    //
    Urb->UrbControlDescriptorRequest.TransferBufferLength = TotalSize;

    //
    // free buffer
    //
    FreeItem(Buffer);

    //
    // done
    //
    return STATUS_SUCCESS;
}

NTSTATUS
USBCCGP_PDOSelectConfiguration(
    PDEVICE_OBJECT DeviceObject, 
    PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    PPDO_DEVICE_EXTENSION PDODeviceExtension;
    PURB Urb;
    PUSBD_INTERFACE_INFORMATION InterfaceInformation;
    ULONG InterfaceInformationCount, Index, InterfaceIndex;
    PUSBD_INTERFACE_LIST_ENTRY Entry;
    ULONG NeedSelect, FoundInterface;

    //
    // get current stack location
    //
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // get device extension
    //
    PDODeviceExtension = (PPDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    //
    // get urb
    //
    Urb = (PURB)IoStack->Parameters.Others.Argument1;
    ASSERT(Urb);

    //
    // is there already an configuration handle
    //
    if (Urb->UrbSelectConfiguration.ConfigurationHandle)
    {
        //
        // nothing to do
        //
        return STATUS_SUCCESS;
    }

    //
    // count interface information
    //
    InterfaceInformationCount = 0;
    InterfaceInformation = &Urb->UrbSelectConfiguration.Interface;
    do
    {
        InterfaceInformationCount++;
        InterfaceInformation = (PUSBD_INTERFACE_INFORMATION)((ULONG_PTR)InterfaceInformation + InterfaceInformation->Length);
    }while((ULONG_PTR)InterfaceInformation < (ULONG_PTR)Urb + Urb->UrbSelectConfiguration.Hdr.Length);

    //
    // check all interfaces
    //
    InterfaceInformation = &Urb->UrbSelectConfiguration.Interface;
    Index = 0;
    Entry = NULL;
    DPRINT1("Count %x\n", InterfaceInformationCount);
    do
    {
        DPRINT1("[USBCCGP] SelectConfiguration Function %x InterfaceNumber %x Alternative %x\n", PDODeviceExtension->FunctionDescriptor->FunctionNumber, InterfaceInformation->InterfaceNumber, InterfaceInformation->AlternateSetting);

        //
        // search for the interface in the local interface list
        //
        FoundInterface = FALSE;
        for(InterfaceIndex = 0; InterfaceIndex < PDODeviceExtension->FunctionDescriptor->NumberOfInterfaces; InterfaceIndex++)
        {
            if (PDODeviceExtension->FunctionDescriptor->InterfaceDescriptorList[InterfaceIndex]->bInterfaceNumber == InterfaceInformation->InterfaceNumber)
            {
                // found interface entry
                FoundInterface = TRUE;
                break;
            }
        }

        if (!FoundInterface)
        {
            //
            // invalid parameter
            //
            DPRINT1("InterfaceInformation InterfaceNumber %x Alternative %x NumberOfPipes %x not found\n", InterfaceInformation->InterfaceNumber, InterfaceInformation->AlternateSetting, InterfaceInformation->NumberOfPipes);
            ASSERT(FALSE);
            return STATUS_INVALID_PARAMETER;
        }

        //
        // now query the total interface list
        //
        Entry = NULL;
        for(InterfaceIndex = 0; InterfaceIndex < PDODeviceExtension->InterfaceListCount; InterfaceIndex++)
        {
            if (PDODeviceExtension->InterfaceList[InterfaceIndex].Interface->InterfaceNumber == InterfaceInformation->InterfaceNumber)
            {
                //
                // found entry
                //
                Entry = &PDODeviceExtension->InterfaceList[InterfaceIndex];
            }
        }

        //
        // sanity check
        //
        ASSERT(Entry);
        if (!Entry)
        {
            //
            // corruption detected
            //
            KeBugCheck(0);
        }

        NeedSelect = FALSE;
        if (Entry->InterfaceDescriptor->bAlternateSetting == InterfaceInformation->AlternateSetting)
        {
            for(InterfaceIndex = 0; InterfaceIndex < Entry->InterfaceDescriptor->bNumEndpoints; InterfaceIndex++)
            {
                if (InterfaceInformation->Pipes[InterfaceIndex].MaximumTransferSize != Entry->Interface->Pipes[InterfaceIndex].MaximumTransferSize)
                {
                    //
                    // changed interface
                    //
                    NeedSelect = TRUE;
                }
            }
        }
        else
        {
            //
            // need select as the interface number differ
            //
            NeedSelect = TRUE;
        }

        if (!NeedSelect)
        {
            //
            // interface is already selected
            //
            ASSERT(InterfaceInformation->Length == Entry->Interface->Length);
            RtlCopyMemory(InterfaceInformation, Entry->Interface, Entry->Interface->Length);
        }
        else
        {
            //
            // FIXME select interface
            //
            UNIMPLEMENTED
            ASSERT(FALSE);
        }

        //
        // move to next information
        //
        InterfaceInformation = (PUSBD_INTERFACE_INFORMATION)((ULONG_PTR)InterfaceInformation + InterfaceInformation->Length);
        Index++;
    }while(Index < InterfaceInformationCount);

    //
    // store configuration handle
    //
    Urb->UrbSelectConfiguration.ConfigurationHandle = PDODeviceExtension->ConfigurationHandle;

    DPRINT1("[USBCCGP] SelectConfiguration Function %x Completed\n", PDODeviceExtension->FunctionDescriptor->FunctionNumber);

    //
    // done
    //
    return STATUS_SUCCESS;
}

NTSTATUS
PDO_HandleInternalDeviceControl(
    PDEVICE_OBJECT DeviceObject, 
    PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    PPDO_DEVICE_EXTENSION PDODeviceExtension;
    NTSTATUS Status;
    PURB Urb;

    //
    // get current stack location
    //
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // get device extension
    //
    PDODeviceExtension = (PPDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    if (IoStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_INTERNAL_USB_SUBMIT_URB)
    {
        //
        // get urb
        //
        Urb = (PURB)IoStack->Parameters.Others.Argument1;
        ASSERT(Urb);
        DPRINT1("IOCTL_INTERNAL_USB_SUBMIT_URB Function %x\n", Urb->UrbHeader.Function);

        if (Urb->UrbHeader.Function == URB_FUNCTION_SELECT_CONFIGURATION)
        {
            //
            // select configuration
            //
            Status = USBCCGP_PDOSelectConfiguration(DeviceObject, Irp);
            Irp->IoStatus.Status = Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return Status;
        }
        else if (Urb->UrbHeader.Function == URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE)
        {
            if(Urb->UrbControlDescriptorRequest.DescriptorType == USB_DEVICE_DESCRIPTOR_TYPE)
            {
                //
                // is the buffer big enough
                //
                if (Urb->UrbControlDescriptorRequest.TransferBufferLength < sizeof(USB_DEVICE_DESCRIPTOR))
                {
                    //
                    // invalid buffer size
                    //
                    DPRINT1("[USBCCGP] invalid device descriptor size %lu\n", Urb->UrbControlDescriptorRequest.TransferBufferLength);
                    Urb->UrbControlDescriptorRequest.TransferBufferLength = sizeof(USB_DEVICE_DESCRIPTOR);
                    Irp->IoStatus.Status = STATUS_INVALID_BUFFER_SIZE;
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);
                    return STATUS_INVALID_BUFFER_SIZE;
                }

                //
                // copy device descriptor
                //
                ASSERT(Urb->UrbControlDescriptorRequest.TransferBuffer);
                RtlCopyMemory(Urb->UrbControlDescriptorRequest.TransferBuffer, &PDODeviceExtension->DeviceDescriptor, sizeof(USB_DEVICE_DESCRIPTOR));
                Irp->IoStatus.Status = STATUS_SUCCESS;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return STATUS_SUCCESS;
            }
            else if (Urb->UrbControlDescriptorRequest.DescriptorType == USB_CONFIGURATION_DESCRIPTOR_TYPE)
            {
                //
                // build configuration descriptor
                //
                Status = USBCCGP_BuildConfigurationDescriptor(DeviceObject, Irp);
                Irp->IoStatus.Status = Status;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                return Status;
            }
        }
        else if (Urb->UrbHeader.Function == URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE)
        {
            DPRINT1("URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE\n");
            IoSkipCurrentIrpStackLocation(Irp);
            Status = IoCallDriver(PDODeviceExtension->NextDeviceObject, Irp);
            DPRINT1("URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE Status %x\n", Status);
            return Status;
        }
    }



    DPRINT1("IOCTL %x\n", IoStack->Parameters.DeviceIoControl.IoControlCode);
    DPRINT1("InputBufferLength %lu\n", IoStack->Parameters.DeviceIoControl.InputBufferLength);
    DPRINT1("OutputBufferLength %lu\n", IoStack->Parameters.DeviceIoControl.OutputBufferLength);
    DPRINT1("Type3InputBuffer %p\n", IoStack->Parameters.DeviceIoControl.Type3InputBuffer);

    ASSERT(FALSE);

    Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
PDO_Dispatch(
    PDEVICE_OBJECT DeviceObject, 
    PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    NTSTATUS Status;

    /* get stack location */
    IoStack = IoGetCurrentIrpStackLocation(Irp);

    switch(IoStack->MajorFunction)
    {
        case IRP_MJ_PNP:
            return PDO_HandlePnp(DeviceObject, Irp);
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
            return PDO_HandleInternalDeviceControl(DeviceObject, Irp);
        default:
            DPRINT1("PDO_Dispatch Function %x not implemented\n", IoStack->MajorFunction);
            ASSERT(FALSE);
            Status = Irp->IoStatus.Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return Status;
    }

}
