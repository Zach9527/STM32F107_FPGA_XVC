#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "tcpserver.h"
#include "string.h"
#include "drvflash.h"
#include "netconf.h"
#include "flash.h"
#include "main.h"
//#include "netif.h"

/* ECHO protocol states */
enum tcp_server_states
{
    ES_NONE = 0,
    ES_ACCEPTED,
    ES_RECEIVED,
    ES_CLOSING
};

struct tcp_server_struct
{
    u8_t state;             /* current connection state */
    struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
    struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};




static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_receive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_server_error(void *arg, err_t err);
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
static void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
static void tcp_server_deal(struct tcp_pcb *tpcb, struct tcp_server_struct *es);

//typedef unsigned char uint8;
static struct tcp_pcb *tcp_server_pcb = NULL;
uint8 tcp_recvbuf[TCP_RECV_BUF_MAX_SIZE];
const uint16_t XVC_ADDR_GPIO[8] = {XVC_ADDR0_PIN,XVC_ADDR1_PIN,XVC_ADDR2_PIN,XVC_ADDR3_PIN,XVC_ADDR4_PIN,
                                XVC_ADDR5_PIN,XVC_ADDR6_PIN,XVC_ADDR7_PIN};
const uint16_t XVC_DATA_GPIO[8] = {XVC_DATA0_PIN,XVC_DATA1_PIN,XVC_DATA2_PIN,XVC_DATA3_PIN,XVC_DATA4_PIN,
                                XVC_DATA5_PIN,XVC_DATA6_PIN,XVC_DATA7_PIN};
extern struct netif netif;

void Tcp_Server_Init(void)
{
    /* create new tcp pcb */
    tcp_server_pcb = tcp_new();
    tcp_server_pcb->so_options |=SOF_KEEPALIVE;
    if (tcp_server_pcb != NULL)
    {
        err_t err;
        /* bind echo_pcb to port */
        err = tcp_bind(tcp_server_pcb, IP_ADDR_ANY, TCP_Port);
        if (err == ERR_OK)
        {
            /* start tcp listening for echo_pcb */
            tcp_server_pcb = tcp_listen(tcp_server_pcb);
    
            /* initialize LwIP tcp_accept callback function */
            tcp_accept(tcp_server_pcb, tcp_server_accept);
        }
        else
        {
            /* deallocate the pcb */
            memp_free(MEMP_TCP_PCB, tcp_server_pcb);
            //printf("Can not bind pcb\n");
        }
    }
    else
    {
        printf("Can not create new pcb\n");
    }
}
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    err_t ret_err;
    struct tcp_server_struct *es;
    
    u32_t ipaddress;
    
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    
    /* set priority for the newly accepted tcp connection newpcb */
    tcp_setprio(newpcb, TCP_PRIO_MIN);
    
    /* allocate structure es to maintain tcp connection informations */
    es = (struct tcp_server_struct *)mem_malloc(sizeof(struct tcp_server_struct));
    if (es != NULL)
    {
        es->state = ES_ACCEPTED;
        es->pcb = newpcb;
        es->p = NULL;
        
        /* pass newly allocated es structure as argument to newpcb */
        tcp_arg(newpcb, es);
        /* initialize lwip tcp_recv callback function for newpcb  */ 
        tcp_recv(newpcb, tcp_server_receive);
        /* initialize LwIP tcp_sent callback function for newpcb */
        tcp_sent(newpcb, tcp_server_sent);
        /* initialize lwip tcp_err callback function for newpcb  */
        tcp_err(newpcb, tcp_server_error);
        /* initialize lwip tcp_poll callback function for newpcb */
        tcp_poll(newpcb, tcp_server_poll, 1);   
        ipaddress = newpcb->remote_ip.addr;
        ret_err = ERR_OK;
    }
    else
    {	
        /*  close tcp connection */
        tcp_server_connection_close(newpcb, es);
        /* return memory error */
        ret_err = ERR_MEM;
    }
    return ret_err;  
}
static err_t tcp_server_receive(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    struct tcp_server_struct *es;
    err_t ret_err;
    LWIP_ASSERT("arg != NULL",arg != NULL);
    es = (struct tcp_server_struct *)arg;
    /* if we receive an empty tcp frame from client => close connection */
    if (p == NULL)
    {
    /* remote host closed connection */
        es->state = ES_CLOSING;
        if(es->p == NULL)
        {
            /* we're done sending, close connection */
            tcp_server_connection_close(tpcb, es);
        }
        else
        {
            /* we're not done yet */
            /* acknowledge received packet */
            tcp_sent(tpcb, tcp_server_sent);
            /* send remaining data*/
            tcp_server_deal(tpcb, es);
        }
        ret_err = ERR_OK;
    }   
    /* else : a non empty frame was received from client but for some reason err != ERR_OK */
    else if(err != ERR_OK)
    {
        /* free received pbuf*/
        es->p = NULL;
        pbuf_free(p);
        ret_err = err;
    }
    else if(es->state == ES_ACCEPTED || es->state == ES_RECEIVED) 
    {
    /* first data chunk in p->payload */
        if(es->state == ES_ACCEPTED)
        {
            es->state = ES_RECEIVED;
        }
    
    /* store reference to incoming pbuf (chain) */
        if(es->p == NULL)
        {
            es->p = p;
            /* send back the received data (echo) */
            tcp_server_deal(tpcb, es);
            ret_err = ERR_OK;
        }
        else
        {
            struct pbuf *ptr;
            /* chain pbufs to the end of what we recv'ed previously  */
            ptr = es->p;
            pbuf_chain(ptr,p);
        }
        ret_err = ERR_OK;
    }
    else if(es->state == ES_CLOSING)
    {
    /* odd case, remote side closing twice, trash data */ 
        tcp_recved(tpcb, p->tot_len);
        es->p = NULL;
        pbuf_free(p);
        ret_err = ERR_OK;
    }
    /* data received when connection already closed */
    else
    {
    /* Acknowledge data reception */
        tcp_recved(tpcb, p->tot_len);
    /* free pbuf and do nothing */
        es->p = NULL;
        pbuf_free(p);
        ret_err = ERR_OK;
    }
    return ret_err;
}


static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_server_struct *es;
    
    LWIP_UNUSED_ARG(len);
    
    es = (struct tcp_server_struct *)arg;
    
    if(es->p != NULL)
    {
    
    /* initialize LwIP tcp_sent callback function */
        tcp_sent(tpcb, tcp_server_sent);
        /* still got pbufs to send */
        tcp_server_deal(tpcb, es);
    }
    else
    {
    /* if no more data to send and client closed connection*/
        if(es->state == ES_CLOSING)
        tcp_server_connection_close(tpcb, es);
    }
    return ERR_OK;
}

static void tcp_server_error(void *arg, err_t err)
{
    struct tcp_server_struct *es;
    
    LWIP_UNUSED_ARG(err);
    
    es = (struct tcp_server_struct *)arg;
    if (es != NULL)
    {
    /*  free es structure */
        mem_free(es);
    }
}
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
    struct tcp_server_struct *es;
    
    es = (struct tcp_server_struct *)arg;
    if (es != NULL)
    {
        if (es->p != NULL)
        {
            tcp_sent(tpcb, tcp_server_sent);
            /* there is a remaining pbuf (chain) , try to send data */
            tcp_server_deal(tpcb, es);
        }
        else
        {
            /* no remaining pbuf (chain)  */
            if(es->state == ES_CLOSING)
            {
            /*  close tcp connection */
            tcp_server_connection_close(tpcb, es);
            }
        }
        ret_err = ERR_OK;
    }
    else
    {
    /* nothing to be done */
        tcp_abort(tpcb);
        ret_err = ERR_ABRT;
    }
    return ret_err;
}

static void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
    u32_t ipaddress; 
    ipaddress = tpcb->remote_ip.addr;
    //printf("client close IP:%d.%d.%d.%d  ", 
    //(u8_t)(ipaddress),(u8_t)(ipaddress >> 8),(u8_t)(ipaddress >> 16),(u8_t)(ipaddress >> 24));
        //printf("port:%d\r\n",tpcb->remote_port);
    /* remove all callbacks */
    tcp_arg(tpcb, NULL);
    tcp_sent(tpcb, NULL);
    tcp_recv(tpcb, NULL);
    tcp_err(tpcb, NULL);
    tcp_poll(tpcb, NULL, 0);
    /* delete es structure */
    if (es != NULL)
    {
        mem_free(es);
    }
    /* close tcp connection */
    tcp_close(tpcb);
}

static void tcp_server_deal(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
  struct pbuf *ptr;
    u16_t length = 0;
    u8_t freed;
    while ((es->p != NULL) && (es->p->len <= tcp_sndbuf(tpcb)))
    { 
          u16_t plen;
      /* get pointer on pbuf from es structure */
      ptr = es->p;
          
          /* handle data */
          memcpy(tcp_recvbuf+length, ptr->payload, ptr->len);  
          length += ptr->len;

      /* enqueue data for transmission */
         //    tcp_write(tpcb, ptr->payload, ptr->len, 1);
      
       plen = ptr->len;
       
        /* continue with next pbuf in chain (if any) */
       es->p = ptr->next;
        
       if(es->p != NULL)
       {
          /* increment reference count for es->p */
         pbuf_ref(es->p);
       }
        
        /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
          do
          {
              freed = pbuf_free(ptr);
           } while (freed == 0);

        /* Update tcp window size to be advertized : should be called when received
        data (with the amount plen) has been processed by the application layer */
       tcp_recved(tpcb, plen);
     }
    XVC_Interface(tpcb, tcp_recvbuf, length);


}


struct tcp_pcb* earsePcb;
ICD_COMMON_RSP_T commonRsp ;
ICD_PARA_T    ipGetRsp;
ICD_PARA_R    ipSetRsp;
ICD_PARA_T    ipSetCmd;
ICD_STATE_RSP_T stateRsp;
extern int earseFlag;
uint8 sendbuf[1024];
//uint8 statebuf[128];
uint32 flashWriteAddr = 0;
uint32 readTotal = 0;
uint32 flashReadAddr = 0;
uint32 ackCnt = 0;
uint8 setAddrFlag = 0;
void XVC_Interface(struct tcp_pcb * tpcb, uint8 * recv,uint16_t length)
{
    int check =0,result = 0,packSize = 0;
    ICD_COMMON_CMD *tcpbuf;
    ICD_WRITE_CMD_T *writeCmd;
    ICD_READ_CMD_T  *readCmd;
    ICD_READ_RSP_T *readRsp;
    uint32* p32;
    readRsp = (ICD_READ_RSP_T *)sendbuf;
    tcpbuf = (ICD_COMMON_CMD*)recv;
    readCmd = (ICD_READ_CMD_T*)recv;
    memcpy(&check,recv+length-4,4);
    tpcb->flags |= TF_NODELAY|TF_ACK_NOW;
    if(tcpbuf->frameHead == TCPHEADFRAME)
    {
        if(check_sum(recv,length-4) == check) //只校验帧头和校验和
        {
            switch(tcpbuf->cmd)
            {
                case ICD_CMD_EARSE:
                    earseFlag = 1;
                    flashChoice(tcpbuf->fpgaCs);
                    XVC_W;
                    XVC_CS_L;
                    Flash_EraseChip();
                    earsePcb = tpcb;
                    //tcp_write(tpcb,(uint8*)(&commonRsp),sizeof(ICD_COMMON_RSP_T),1);
                    XVC_CS_H;
                    break;
                case ICD_CMD_WRITE:
                    writeCmd = (ICD_WRITE_CMD_T *)recv;
                    if(setAddrFlag == 0)
                    {
                        flashChoice(writeCmd->fpgaCs);
                        XVC_W;
                        XVC_CS_L;
                        setAddrFlag = 1;
                        XVC_CS_H;
                    }
                    if((writeCmd->size-8 )!= 0)
                    {
                        result = Flash_WriteNoCheck(writeCmd->dat,
                                            flashWriteAddr,writeCmd->size - 8);//写入数据
                        flashWriteAddr += writeCmd->size - 8;
                    }
                    else
                    {
                        writeCntInfo(&flashWriteAddr); //将数据存入片上flash
                        result = 1;
                        flashWriteAddr = 0;
                        setAddrFlag = 0;
                    }
                    //响应数据
                    commonRsp.head = TCPHEADFRAME;
                    commonRsp.cmd = ICD_CMD_WRITE;
                    commonRsp.size = 4;
                    commonRsp.opRet = result;
                    commonRsp.tail = TCPTAILFRAME;
                    commonRsp.checkSum = check_sum((uint8*)(&commonRsp),sizeof(ICD_COMMON_RSP_T)-4);
                    tcp_write(tpcb,(uint8*)(&commonRsp),sizeof(ICD_COMMON_RSP_T),1);
                    break;
                case ICD_CMD_READ:
                    if(setAddrFlag == 0)
                    {
                        flashChoice(tcpbuf->fpgaCs);
                        XVC_W;
                        XVC_CS_L;
                        setAddrFlag = 1;
                        readTotal = readCntInfo();
                        XVC_CS_H;
                        if(readTotal%ICD_DAT_MAX_SIZE)
                            ackCnt = readTotal/ICD_DAT_MAX_SIZE + 2;
                        else
                            ackCnt = readTotal/ICD_DAT_MAX_SIZE + 1;
                    }
                    readRsp->head = TCPHEADFRAME;
                    readRsp->cmd = ICD_CMD_READ;
                    if(readTotal > ICD_DAT_MAX_SIZE)
                    {
                        Flash_ReadSomeByte(readRsp->dat,flashReadAddr,ICD_DAT_MAX_SIZE);
                        readRsp->size = ICD_DAT_MAX_SIZE;
                        flashReadAddr += ICD_DAT_MAX_SIZE;
                        readTotal -= ICD_DAT_MAX_SIZE;
                    }
                    else if(ackCnt >= readCmd->ackCnt && readTotal < ICD_DAT_MAX_SIZE)
                    {
                        Flash_ReadSomeByte(readRsp->dat,flashReadAddr,readTotal);
                        readRsp->size = readTotal;
                        if(readTotal == 0)
                        {
                            setAddrFlag = 0;
                            flashReadAddr = 0 ;
                            ackCnt = 0;
                        }
                        readTotal = 0;
                    }
                    p32 = (uint32 *)(readRsp->dat + readRsp->size);
                    *p32 = TCPTAILFRAME;
                    p32 += 1;
                    packSize = readRsp->size + 20;
                    *p32 = check_sum(sendbuf,packSize-4);
                    tcp_write(tpcb,sendbuf,packSize,1);
                    break;
                case ICD_CMD_PARA_GET:
                    ipGetRsp.head = TCPHEADFRAME;
                    ipGetRsp.cmd = ICD_CMD_PARA_GET;
                    ipGetRsp.size = 12;
                    ipaddrGet((uint32)netif.ip_addr.addr,ipGetRsp.ip);
                    macaddrGet(netif.hwaddr,ipGetRsp.mac);
                    ipGetRsp.port = netCfgInfo.serverPort;
                    ipGetRsp.tail = TCPTAILFRAME;
                    ipGetRsp.checkSum = check_sum((uint8*)(&ipGetRsp),sizeof(ICD_PARA_T)-4);
                    tcp_write(tpcb,(uint8*)(&ipGetRsp),sizeof(ICD_PARA_T),1);
                    break;
                case ICD_CMD_PARA_SET:
                    memcpy(&ipSetCmd,recv,length);
                    ipSetRsp.head =TCPHEADFRAME;
                    ipSetRsp.cmd = ICD_CMD_PARA_SET;
                    ipSetRsp.size = sizeof(ipSetRsp.result);
                    result = ipaddrSet(ipSetCmd.ip);
                    if(result)
                    {
                        result = macaddrSet(ipSetCmd.mac);
                    }
                    if(result)
                    {
                        result = portSet(ipSetCmd.port);
                    }
                    ipSetRsp.result = result;
                    ipSetRsp.tail = TCPTAILFRAME;
                    ipSetRsp.checkSum = check_sum((uint8*)(&ipSetRsp),sizeof(ICD_PARA_R)-4);
                    tcp_write(tpcb,(uint8*)(&ipSetRsp),sizeof(ICD_PARA_R),1);
                    break;
                case ICD_CMD_STATE_GET:
                    stateRsp.head = TCPHEADFRAME;
                    stateRsp.cmd = ICD_CMD_STATE_GET;
                    stateRsp.size = sizeof(stateRsp.dat);
                    fpgaStateGet(stateRsp.dat);
                    stateRsp.tail = TCPTAILFRAME;
                    stateRsp.checkSum = check_sum((uint8*)(&stateRsp),sizeof(ICD_STATE_RSP_T)-4);
                    tcp_write(tpcb,(uint8*)(&stateRsp),sizeof(ICD_STATE_RSP_T),1);
                    break;
                default:
                    break;
            }
        }
    }

}
void setAddressLine(uint8 addr)
{
    int i;
    char addrSelect = 0;
    for(i = 0; i < 8; i++)
    {
        addrSelect = (addr >> i)& 0x01;
        if(addrSelect)
        {
            GPIO_SetBits(GPIOE,XVC_ADDR_GPIO[i]);

        }
        else
        {
            GPIO_ResetBits(GPIOE,XVC_ADDR_GPIO[i]);

        }
    }
}

void setDataLine(uint8 data)
{
    int i;
    char dataSelect = 0;
    for(i = 0; i < 8; i++)
    {
        dataSelect = (data >> i) & 0x01;
        if(dataSelect)
        {
            GPIO_SetBits(GPIOE,XVC_DATA_GPIO[i]);
        }
        else
        {
            GPIO_ResetBits(GPIOE,XVC_DATA_GPIO[i]);
        }
    }
}


uint8 getDataLine(void)
{
    int i;
    uint8 read_dat = 0;
    for(i = 0; i < 8; i++)
    {
        read_dat |= (GPIO_ReadInputDataBit(GPIOE, XVC_DATA_GPIO[i])<<i);
    }
    return read_dat;
}
int flashChoice(int select)
{
    setAddressLine(0x60);
    switch(select)
    {
        case 0x01:
            setDataLine(0x01);
            break;
        case 0x02:
            setDataLine(0x02);
            break;
        case 0x03:
            setDataLine(0x03);
            break;
        default:
        break;
    }
}
uint32 check_sum(uint8* pbuf,int len)
{
    int i = 0;
    uint32 checksum = 0;
    for(i=0;i<len;i++)
    {
        checksum += pbuf[i];
    }
    return checksum;
}


void ipaddrGet(uint32 ip, uint8 *str)
{
    //uint32 ipTmp;
    //ipTmp = PP_HTONL(ip);
    memcpy(str,&ip,sizeof(uint32));
}
void macaddrGet(uint8 *mac, uint8 *str)
{
    memcpy(str,mac,6);
}


int ipaddrSet(uint8_t *cmd)
{
    int sta;
    memcpy(ipInfoFlash.ip,cmd,4);
    ipInfoFlash.ipValid = DATA_VALID_VAL;
    sta = writeIpInfo(&ipInfoFlash);
    if(FLASH_OK != sta)
    {
            //状态返回
        return 0;
    }
    else
    {
        return 1;//状态返回
    }
}

int macaddrSet(uint8_t *cmd)
{
    int sta;
    memcpy(ipInfoFlash.mac,cmd,6);
    ipInfoFlash.macValid = DATA_VALID_VAL;
    sta = writeIpInfo(&ipInfoFlash);
    if(FLASH_OK != sta)
    {
        return 0;
    }else
    {
        return 1;
    }
}
 int portSet(uint16 port)
{
    int sta;
    memcpy(ipInfoFlash.port, &port,2);
    ipInfoFlash.portValid = DATA_VALID_VAL;
    sta = writeIpInfo(&ipInfoFlash);
    if(FLASH_OK != sta)
    {
        return 0;
    }else
    {
        return 1;
    }
}

  static uint8 BCD2DEC(uint8 bcd)
  {
      return (bcd - (bcd >> 4) * 6);
  }



void fpgaStateGet(uint8* sbuf)
{
    int i;
    uint8 dat,*buf;
    buf = sbuf;
    //时间获取
    for(i = 0x4;i < 0x0a;i++)
    {
        setAddressLine(i);
        XVC_R;
        XVC_CS_L;
        dat = getDataLine();
        *buf = BCD2DEC(dat);
        buf++;
        XVC_CS_H;
    }
    //状态获取
    for(i = 0x10;i < 0x13;i++)
    {
        setAddressLine(i);
        XVC_R;
        XVC_CS_L;
        *buf = getDataLine();
        buf++;
        XVC_CS_H;
    }
    //二次电源状态获取
    for(i = 0x14;i < 0x18;i++)
    {
        setAddressLine(i);
        XVC_R;
        XVC_CS_L;
        *buf = getDataLine();
        buf++;
        XVC_CS_H;
    }
    //温度获取
    for(i = 0x20; i < 0x30;i++)
    {
        setAddressLine(i);
        XVC_R;
        XVC_CS_L;
        *buf = getDataLine();
        buf++;
        XVC_CS_H;
    }
    //电压获取
    for(i = 0x30; i < 0x40;i++)
    {
        setAddressLine(i);
        XVC_R;
        XVC_CS_L;
        *buf = getDataLine();
        buf++;
        XVC_CS_H;
    }
    //时钟状态获取
    for(i = 0x50;i < 0x52;i++)
    {
        setAddressLine(i);
        XVC_R;
        XVC_CS_L;
        *buf = getDataLine();
        buf++;
        XVC_CS_H;
    }
    //外部上电状态获取
    setAddressLine(0x54);
    XVC_R;
    XVC_CS_L;
    *buf = getDataLine();
    XVC_CS_H;
}


