/*********************************************************************************
 * This confidential and proprietary software may be used only as authorized 
 *                      by a licensing agreement from                           
 *                           Cortus S.A.
 *
 *             (C) Copyright 2004, 2005, 2006 Cortus S.A.
 *                           ALL RIGHTS RESERVED
 *
 * The entire notice above must be reproduced on all authorized copies
 * and any such reproduction must be pursuant to a licensing agreement 
 * from Cortus S.A. (http://www.cortus.com)
 *
 * $CortusRelease$
 * $FileName$
 *
 *********************************************************************************/

#ifndef _ETHERNET_H
#define _ETHERNET_H
#include <machine/sfradr.h>

typedef struct ETH_MAC
{
    volatile unsigned sw_reset;         
    volatile unsigned addr_low;          
    volatile unsigned addr_high;          
    volatile unsigned max_frame_size;      
    volatile unsigned collision_window;  
    volatile unsigned max_collision;      
    volatile unsigned interframe_gap;     
    volatile unsigned max_deferral;        
    volatile unsigned hash_filter_low;    
    volatile unsigned hash_filter_high; 
    volatile unsigned mac_status;          
    volatile unsigned full_duplex;        
    volatile unsigned no_padding;         
    volatile unsigned crc_disable;          
    volatile unsigned no_backoff;             
    volatile unsigned unicast;            
    volatile unsigned multicast;         
    volatile unsigned broadcast;          
    volatile unsigned hash;                
    volatile unsigned exact_addr;          
    volatile unsigned indefinite_deferral; 
} ETH_MAC;

typedef struct ETH_MIIM
{
    volatile unsigned miim_status;           
    volatile unsigned miim_phy_addr;           
    volatile unsigned miim_phy_register_addr;   
    volatile unsigned miim_clock_divider;      
    volatile unsigned miim_read_write;         
    volatile unsigned miim_data;                 
} ETH_MIIM;

typedef struct ETH_RX
{
    volatile unsigned rx_enable;    
    volatile unsigned rx_desc_status;     
    volatile unsigned rx_desc_base_addr;    
    volatile unsigned rx_desc_number;      
    volatile unsigned rx_desc_produce;        
    volatile unsigned rx_desc_consume;  
    volatile unsigned rx_threshold;    
    volatile unsigned rx_sw_done;          
    volatile unsigned rx_irq_mask;            
    volatile unsigned rx_status;
} ETH_RX;

typedef struct ETH_TX
{
    volatile unsigned tx_enable;              
    volatile unsigned tx_desc_status;         
    volatile unsigned tx_desc_base_addr;      
    volatile unsigned tx_desc_number;         
    volatile unsigned tx_desc_produce;        
    volatile unsigned tx_desc_consume; 
    volatile unsigned tx_threshold;         
    volatile unsigned tx_sw_done;             
    volatile unsigned tx_irq_mask;            
    volatile unsigned tx_status;             
} ETH_TX;


#ifdef __APS__
#define eth_mac ((ETH_MAC *)SFRADR_ETH_MAC)
#else
extern eth_mac __eth_mac;
#define eth_mac (&__eth_mac)
#endif

#ifdef __APS__
#define eth_miim ((ETH_MIIM *)SFRADR_ETH_MIIM)
#else
extern eth_miim __eth_miim;
#define eth_miim (&__eth_miim)
#endif

#ifdef __APS__
#define eth_rx ((ETH_RX *)SFRADR_ETH_RX)
#else
extern eth_rx __eth_rx;
#define eth_rx (&__eth_rx)
#endif

#ifdef __APS__
#define eth_tx ((ETH_TX *)SFRADR_ETH_TX)
#else
extern eth_tx __eth_tx;
#define eth_tx (&__eth_tx)
#endif
#endif
