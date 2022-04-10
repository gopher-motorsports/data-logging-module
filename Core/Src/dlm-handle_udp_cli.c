// dlm-handle_udp_cli.c
//  This file will handle communication over the UDP socket to a
//  client in order to send and receive messages over UDP. Callbacks
//  can be added into the command table along with function pointers
//  to allow specific code be be run by a user. There is also ETH_printf
//  functionality in order to send generic data to the user


#include "dlm-handle_udp_cli.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include <string.h>

static struct netconn *conn;
static struct netbuf *buf;
static unsigned short port;
static ip_addr_t *addr;
char msg[100];
char smsg[200];


// the command table
// TODO


// ETH_printf
// TODO


// handle_udp_cli_init
void handle_udp_cli_init(void)
{
	// TODO maybe delete this if it isnt needed for anything
}


// handle_udp_cli
// TODO DOCS
void handle_udp_cli(void)
{
	// TODO change this to be much better
	err_t err, recv_err;
	struct pbuf *txBuf;

	/* Create a new connection identifier */
	conn = netconn_new(NETCONN_UDP);

	if (conn!= NULL)
	{
		/* Bind connection to the port 7 */
		err = netconn_bind(conn, IP_ADDR_ANY, 7);

		if (err == ERR_OK)
		{
			/* The while loop will run everytime this Task is executed */
			while (1)
			{
				/* Receive the data from the connection */
				recv_err = netconn_recv(conn, &buf);

				if (recv_err == ERR_OK) // if the data is received
				{
					addr = netbuf_fromaddr(buf);  // get the address of the client
					port = netbuf_fromport(buf);  // get the Port of the client
					strcpy (msg, buf->p->payload);   // get the message from the client

					// Or modify the message received, so that we can send it back to the client
					int len = sprintf (smsg, "\"%s\" was recieved by the DLM\n", (char *) buf->p->payload);

					/* allocate pbuf from RAM*/
					txBuf = pbuf_alloc(PBUF_TRANSPORT,len, PBUF_RAM);

					/* copy the data into the buffer  */
					pbuf_take(txBuf, smsg, len);

					// refer the nebuf->pbuf to our pbuf
					buf->p = txBuf;

					netconn_connect(conn, addr, port);  // connect to the destination address and port

					netconn_send(conn,buf);  // send the netbuf to the client

					buf->addr.addr = 0;  // clear the address
					pbuf_free(txBuf);   // clear the pbuf
					netbuf_delete(buf);  // delete the netbuf
				}

				osDelay(1);
			}
		}
		else
		{
			netconn_delete(conn);
		}
	}
}
