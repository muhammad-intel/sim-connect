/*
  forward-receiver.c - Skeleton code to base new device modules on
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <simics/device-api.h>

#include <simics/model-iface/transaction.h>
#include "cxl_tlp_fifo.h"
typedef struct {
   // Simics configuration object
   conf_object_t obj;

   // Dummy return value to use in this demo
   // Use lower x bytes of this value for <8 byte transactions
   uint64                   dummy_return_value;
   bool                     use_rtl;
   bool                     mem_access_type;
} forwarder_device_t;
static simics_transaction_t send_pkt,reply_pkt;
extern int simics_send_access_to_cosim(void *send_data,void *reply_data, int verbose, int time_start, int time_end);
int verbose=0, time_start_flag=0, time_end_flag=0;
/* Allocate memory for the object. */
static conf_object_t *
alloc_object(conf_class_t *cls)
{
   forwarder_device_t *dev = MM_ZALLOC(1, forwarder_device_t);
   return &dev->obj;
}

/* Initialize the object before any attributes are set. */
static void *
init_object(conf_object_t *obj)
{
   // Initialize default value for attribute. 
   // Checkpoints & configuration setup code will override.
   forwarder_device_t *dev = (forwarder_device_t *) obj;
   dev->dummy_return_value = 0xfedcba9876543210;
   dev->use_rtl            = false;
   dev->mem_access_type    = false;

   return obj;
}

/* Finalize the object after attributes have been set, if needed. */
static void
finalize_object(conf_object_t *obj)
{
   /* USER-TODO: Add initialization code here that has to run after the
      attribute setters but that does not communicate with other objects
      or post events */
}

/* Initialization once all objects have been finalized, if needed. */
static void
objects_finalized(conf_object_t *obj)
{
   /* USER-TODO: Add initialization code here that communicates with other
      objects or posts events */
}

/* Called during object deletion while other objects may still be accessed. */
static void
deinit_object (conf_object_t *obj)
{
   /* USER-TODO: Remove all external references that this object has set
      up to itself, for example, breakpoints and hap callbacks */
}

/* Free memory allocated for the object. */
static void
dealloc_object(conf_object_t *obj)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   /* USER-TODO: Free any memory allocated for the object */
   MM_FREE(dev);
}

//----------------------------------------------------------------------
//
// Incoming memory operations hit this call! 
// 
//----------------------------------------------------------------------
static exception_type_t
issue(conf_object_t *obj, transaction_t *t, uint64 addr)
{
  // Compute dummy value considering alignment and size
  uint64 mask;
  int    align;
  uint64 shifted_val;
  uint64 return_val;

  forwarder_device_t *dev = (forwarder_device_t *)obj;

  // Do anything with the accesses here!
  int opsize              = SIM_transaction_size(t);
        
  if (dev->use_rtl) {
    memset(&send_pkt,0,sizeof(simics_transaction_t));
    memset(&reply_pkt,0,sizeof(simics_transaction_t));
  }
	  
  if (SIM_transaction_is_read(t)) {

    // Compute dummy value considering alignment and size
    mask  = 0xffffffffffffffff >> (64-opsize*8);
    align = addr & 0x7;

    if (dev->use_rtl) {
      if (dev->mem_access_type == false)
        send_pkt.packet_type    = 1;
      else
        send_pkt.packet_type    = 2;	
      send_pkt.sim_type         = 1;
      send_pkt.physical_address = addr;
      send_pkt.r0w1             = 0;
      send_pkt.data_size        = opsize;   
      time_start_flag=1;
      simics_send_access_to_cosim(&send_pkt, &reply_pkt,verbose,time_start_flag,time_end_flag);
      /* relay the replied data back to simics */
      memcpy(&return_val,&reply_pkt.data[0],sizeof(uint64));
      SIM_LOG_INFO(2, &dev->obj, 0, 
                   "sizeof(uint64) =%ld, return_val before shift 0x%llx  data[0]=%x,data[1]=%x,data[2]=%x,data[3]=%x,data[4]=%x,data[5]=%x,data[6]=%x,data[7]=%x ", sizeof(uint64),
                   return_val,reply_pkt.data[0],reply_pkt.data[1],reply_pkt.data[2],reply_pkt.data[3],reply_pkt.data[4],reply_pkt.data[5],reply_pkt.data[6],reply_pkt.data[7]);
      // bring the read value in least significant bytes
      shifted_val = return_val >> align*8;
      return_val  = shifted_val & mask;
       SIM_LOG_INFO(2, &dev->obj, 0, 
                   "return_val after shift 0x%llx", 
                   return_val);     
      SIM_set_transaction_value_le(t, return_val);

    } else {

      // Compute dummy value considering alignment and size
      shifted_val = dev->dummy_return_value >> (align*8);
      return_val  = shifted_val & mask;

      SIM_LOG_INFO(2, &dev->obj, 0, 
                   "Read from offset 0x%llx, size %d, returning dummy (0x%llx)", 
                   addr, opsize, return_val);
      SIM_set_transaction_value_le(t, return_val);

      SIM_LOG_INFO(2, &dev->obj, 0, 
                   "use_rtl = %d mem_access_type=%d", 
                   dev->use_rtl, dev->mem_access_type);
    }
                
  } else {
    uint64 val                  = SIM_get_transaction_value_le(t);
    if (dev->use_rtl) {
      if (dev->mem_access_type  == false)
        send_pkt.packet_type     = 1;
      else				
        send_pkt.packet_type    = 2;
      send_pkt.sim_type         = 1;
      send_pkt.physical_address = addr;
      send_pkt.r0w1             = 1;
      send_pkt.data_size        = opsize;
      memcpy(&send_pkt.data[0], &val, sizeof(uint64) );	
      simics_send_access_to_cosim(&send_pkt, &reply_pkt,verbose,time_start_flag,time_end_flag); 	
    } else {	
				
      SIM_LOG_INFO(2, &dev->obj, 0, 
                   "Write to offset 0x%llx, size %d, 0x%llx", 
                   addr, opsize, val);
    }
  }                             //write
  return Sim_PE_No_Exception;
}

//--------------------------------------------------------------------------
//
// Example: manage the value, use_rtl and memory_access_type attribute
// 
//--------------------------------------------------------------------------

/* ------------------------------------------------------------------------ */
static set_error_t
set_value_attribute(conf_object_t *obj, attr_value_t *val)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   dev->dummy_return_value = SIM_attr_integer(*val);
   return Sim_Set_Ok;
}
static attr_value_t
get_value_attribute(conf_object_t *obj)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   return SIM_make_attr_uint64(dev->dummy_return_value);
}

/* ------------------------------------------------------------------------ */
static set_error_t
set_value_attribute_use_rtl(conf_object_t *obj, attr_value_t *val)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   dev->use_rtl            = SIM_attr_boolean(*val);
   return Sim_Set_Ok;
}
static attr_value_t
get_value_attribute_use_rtl(conf_object_t *obj)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   return SIM_make_attr_boolean(dev->use_rtl);
}

/* ------------------------------------------------------------------------ */
static set_error_t
set_value_attribute_mem_access_type(conf_object_t *obj, attr_value_t *val)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   dev->mem_access_type    = SIM_attr_boolean(*val);
   return Sim_Set_Ok;
}

static attr_value_t
get_value_attribute_mem_access_type(conf_object_t *obj)
{
   forwarder_device_t *dev = (forwarder_device_t *)obj;
   return SIM_make_attr_boolean(dev->mem_access_type);
}

/* ------------------------------------------------------------------------ */

/* Called once when the device module is loaded into Simics. */
void
init_local(void)
{
   /* Define and register the device class. */
   const class_info_t class_info = {
      .alloc                     = alloc_object,
      .init                      = init_object,
      .finalize                  = finalize_object,
      .objects_finalized         = objects_finalized,
      .deinit                    = deinit_object,
      .dealloc                   = dealloc_object,
      .description               = "This is a long description of this class.",
      .short_desc                = "single line class description",
      .kind                      = Sim_Class_Kind_Vanilla
   };
   /* USER-TODO: Set the name of the device class */
   conf_class_t *class           = SIM_create_class("forward_receiver", &class_info);

   /* Register the 'transaction' interface, which is the
      interface that is implemented by memory mapped devices. */
   static const transaction_interface_t transaction_iface = {
      .issue                                              = issue,
   };
   SIM_REGISTER_INTERFACE(class, transaction, &transaction_iface);

   /* USER-TODO: Add any attributes for the device here */

   SIM_register_attribute(
      class, "dummy_return_value",
      get_value_attribute, set_value_attribute,
      Sim_Attr_Optional, "i",
      "Value to return from memory reads, in lieu of having something else to talk to.");

   SIM_register_attribute(
      class, "use_rtl",
      get_value_attribute_use_rtl, set_value_attribute_use_rtl,
      Sim_Attr_Optional, "b",
      "If true, runs RTL simulation");

   SIM_register_attribute(
      class, "mem_access_type",
      get_value_attribute_mem_access_type, set_value_attribute_mem_access_type,
      Sim_Attr_Optional, "b",
      "if true, config R/W and if false, mem R/W");
}
