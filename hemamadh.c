#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/select.h>
#include <ifaddrs.h>
#include <ctype.h>
#define MAX_INT 32600
char localip[32];
int packet_count;
int16_t adjmatrix_neigh[6][6];
int myid;
int ID_count;
NODE node[6];
char myIP[32];
int myport;
routing_table rtable[6];
//routing_table neigh_table[6];
int socket1;
int neighbour_count;
int myneigh_id[6];
int packet_first=8;
int packet_second=12;
int dvcount;



char* concatinate_msg(int flag,int16_t cost)
{
    
    //message format
    int16_t x;
    int write_bytes=0;
    int packet_size=packet_first+(packet_second*ID_count);
    char *packet=(char*)malloc(packet_size+2);
    //copy first part of packet;
    memset(packet,1,packet_size);
    
    int16_t temp=htons(ID_count);
    memcpy(packet+write_bytes,&temp,2);
    write_bytes=write_bytes+2;
    
    temp=htons(node[1].port);
    memcpy(packet+write_bytes,&temp,2);
    write_bytes=write_bytes+2;
    
    int32_t temp_ip=node[1].IP;
    
    memcpy(packet+write_bytes,&temp_ip,4);
    write_bytes=write_bytes+4;
    
    int i=1;
    
    while(i<=ID_count)
    {
        
        
        memcpy(packet+write_bytes, &node[i].IP, 4);
        write_bytes = write_bytes+4;
        
        temp= htons(node[i].port);
        memcpy(packet+write_bytes, &temp, 2);
        write_bytes = write_bytes+2;
        
        if(flag==1)
        {
            x=cost;
        }
        else
            x=654;

        temp= htons(x);
        memcpy(packet+write_bytes, &temp, 2);
        write_bytes = write_bytes+2;
        
        temp= htons(node[i].id);
        memcpy(packet+write_bytes, &temp, 2);
        write_bytes = write_bytes+2;
        
        temp= htons(rtable[i].cost);
        memcpy(packet+write_bytes, &temp, 2);
        write_bytes = write_bytes+2;
        i++;
        
    }
    temp= htons(800);
    memcpy(packet+write_bytes, &temp, 2);
    write_bytes = write_bytes+2;
    
    return packet;
    
    
}
void update_specific(int index,int16_t cost)
{
    struct in_addr net_IP;
    struct sockaddr_in server_addr;
    net_IP.s_addr=node[index].IP;
    server_addr.sin_family=AF_INET;
    inet_pton(AF_INET,inet_ntoa(net_IP), &(server_addr.sin_addr));
    server_addr.sin_port=htons(node[index].port);
 char* packet=concatinate_msg(1,cost);
    sendto(socket1, packet,70, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
   
  
}

void display_topology()
{

printf("%-6s%-36s%-6s%-6s%-6s\n", "ID", "IP", "PORT","COST","NEIGHBOUR");
//struct in_addr IP_Net;

for(int i=1;i<=ID_count;i++)
    {  
    struct in_addr IP_Net;
    IP_Net.s_addr=node[i].IP;
    printf("%-6d%-36s%-6d%-6d%-6d\n",node[i].id,inet_ntoa(IP_Net),node[i].port,node[i].cost,node[i].isNeighbour);

    
    }
   

}

void display_routing_table()
{
    int16_t temp_adj_cost[(int16_t)ID_count+1],temp_adj_nh[(int16_t)ID_count+1];
    

    for (int i=1;i<=ID_count;i++)
    {
        temp_adj_cost[rtable[i].id]=rtable[i].cost;
        temp_adj_nh[rtable[i].id]=rtable[i].nexthop;
    }
    printf("This Server has ID:%d\n",myid);
    printf("%-6s%-10s%-9s \n", "ID", "NEXT_HOP", "COST");

    for(int i=1;i<=ID_count;i++)
    {
    //printf("%-6d%-12d%-9d\n",i,temp_adj_nh[i],temp_adj_cost[i]);
     if(temp_adj_cost[i]<MAX_INT)
     printf("%-6d%-12d%-9d\n",i,temp_adj_nh[i],temp_adj_cost[i]);
     else
     printf("%-6d%-12d%-9s\n",i,temp_adj_nh[i],"inf");

    }

}

void get_neighbours()
{
    for(int i=2;i<=ID_count;i++)
    {
         if(node[i].updatecount>3)
           { node[i].isNeighbour=0;
             node[i].cost=MAX_INT;
             rtable[i].nexthop=-1;
             rtable[i].cost=MAX_INT;

           }
    }

        int k=0;
    for (int i=2;i<=ID_count;i++)
    {
         if(node[i].isNeighbour)
         {
           k++;
           myneigh_id[k]=node[i].id;
           


         }
       

    }

    dvcount=k;



}

void sendUpdate()
{
    struct in_addr net_IP;
    char *buffer=concatinate_msg(0,0);
    struct sockaddr_in server_addr;

    get_neighbours();


    for(int i=2;i<=ID_count;i++)
    {
        if(node[i].isNeighbour)
        {
            net_IP.s_addr=node[i].IP;
            server_addr.sin_family=AF_INET;
            inet_pton(AF_INET,inet_ntoa(net_IP), &(server_addr.sin_addr));
            server_addr.sin_port=htons(node[i].port);
            int send_len=sendto(socket1, buffer,70, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if(send_len>0)
                printf("DV Sent SUCCESS to Server ID: %d\n",node[i].id);
        }
        
        
    }
    
    
}



void update_link(int16_t id,int16_t cost)
{
     int index;
    for (int i=1;i<=ID_count;i++)
    {
        if(node[i].id==id)
        {
        
            node[i].cost=cost;
            rtable[i].nexthop=-1;
            rtable[i].cost=MAX_INT;
            index=i;
            break;
            
        
        }
    
    
    }

    update_specific(index,cost);

    printf("Cost Update SUCCESS: Sent to Server:%d\n",node[index].id);


}

void reset_routingtable()
{

  for (int i=2;i<=ID_count;i++)
  {

    rtable[i].cost=MAX_INT;
    rtable[i].nexthop=-1;


  }

}


void distance_vector()
{

reset_routingtable();   
for(int j=1;j<=dvcount;j++)

  {  int16_t fromServID=myneigh_id[j];
      int16_t costtoID;
      for(int i=1;i<=ID_count;i++)
      {
        if(fromServID==node[i].id)
            costtoID=node[i].cost;


      }
  


for (int i=1;i<=ID_count;i++)
    {
    
       if(costtoID+adjmatrix_neigh[(int)fromServID][(int)rtable[i].id]<rtable[i].cost)
       {
       
           rtable[i].cost=costtoID+adjmatrix_neigh[(int)fromServID][(int)rtable[i].id];
           rtable[i].nexthop=fromServID;
       
       
       }
    
    }

}



}

int deconcatinate_msg(char* packet)
{
    int read_bytes=0;
    int j=1;
    int index;
    int32_t serverip;
    int count=0;
   
    int16_t fields,port,servID,cost,dummy,fromServID;
    
    memcpy(&fields, packet+read_bytes, 2);
    fields=ntohs(fields);
    read_bytes = read_bytes+2;
    
    memcpy(&port, packet+read_bytes, 2);
    port=ntohs(port);
    read_bytes = read_bytes+2;
    
    memcpy(&serverip, packet+read_bytes, 4);
    read_bytes = read_bytes+4;
    
    while(j<=(int)fields)
    {
        
        if(node[j].IP==serverip && node[j].port==port && node[j].isNeighbour){
            index=j;
            count++;
            node[index].updatecount=0;
            fromServID=node[j].id;
            break;
        }
        
        j++;
        
        
    }


    if(count==0)
        return 0;
    
    
    
    j=1;
    
    while(j<=(int)fields)
    {
        
        
        memcpy(&serverip, packet+read_bytes, 4);
        read_bytes = read_bytes+4;
        
        memcpy(&port, packet+read_bytes, 2);
        port=ntohs(port);
        read_bytes = read_bytes+2;
        
        memcpy(&dummy, packet+read_bytes, 2);
        dummy=ntohs(dummy);
        read_bytes = read_bytes+2;
        if(dummy!=654)
        {
        
        node[index].cost=dummy;
       
        printf("Link cost update SUCCESS received from server ID:%d\n",node[index].id);

        return 0;

        }


        memcpy(&servID, packet+read_bytes, 2);
        servID=ntohs(servID);
        read_bytes = read_bytes+2;
        
        memcpy(&cost, packet+read_bytes, 2);
        cost=ntohs(cost);
        read_bytes = read_bytes+2;
       
        adjmatrix_neigh[(int)fromServID][(int)servID]=cost;
        
        j++;
        
    }
    
distance_vector();
//display_routing_table();

    return 0;

}

void display_DV_count()
{

   printf("Packet update received count %d\n",packet_count );



}

int Index(int16_t neigh)
{
    
    for (int i=1;i<=ID_count;i++)
    {
        
        if(node[i].id==neigh)
            return i;
        
    }
    return 0;
    
}

void get_ipaddr()
{
    char tempip[32];
    struct ifaddrs *interfaceaddress,*tempaddr;
    getifaddrs(&interfaceaddress);
    for (tempaddr=interfaceaddress;tempaddr!=NULL;tempaddr=tempaddr->ifa_next) {
        if(tempaddr->ifa_addr->sa_family == AF_INET) {
            getnameinfo(tempaddr->ifa_addr, sizeof(struct sockaddr_in),tempip,32, NULL, 0,NI_NUMERICHOST);
            if(strcmp(tempip,"127.0.0.1")!=0)
            {
                if(strstr(tempip,"128"))
                {
                    strcpy(localip,tempip);
                }
                
                
            }
            
        }
        printf("Server is running at %s:",localip);
 
    }
}

int main(int argc, char* argv[]){
    char* file_name;
    int routing_time;
    char file_line[256];
    for (int i = 1; i < argc; i++) {
        if (i + 1 != argc)    {
            if (strcmp(argv[i],"-i")==0) {
                
                
                routing_time = atoi(argv[i + 1]);
            }
            if (strcmp(argv[i],"-t")==0) {
                
                file_name = argv[i + 1];   
            }
        }
    }
    
    FILE* file = fopen(file_name, "r");
    
    if(file==NULL)
    {
        printf("%s\n","Error opening file. File doesn't exist" );
        return -1;
        
    }
    int count=1;
    char ID[4];
    char myID[4];
    char IP[32];
    char neighbour[4];
    char cost[4];
    char port[6];
    int j=2;

    
    ip_port temp_str[6];
    while(fgets(file_line,256,file)!=NULL)
    {
        
        if(count==1)
        {
            ID_count=atoi(file_line);
            
            
            
        }
        else if(count==2)
        {
            
            neighbour_count=atoi(file_line);
           
            

        }
        
        
        else if(count>2 && count < 3+ID_count )
        {
            
            strcpy(ID,strtok(file_line , " "));
            strcpy(IP,strtok(NULL , " "));
            strcpy(port,strtok(NULL , " "));
            strcpy(temp_str[atoi(ID)].ip,IP);
            temp_str[atoi(ID)].port=atoi(port);

            
        }
        
        else if(count>= 3+ID_count && count < 3+ID_count+neighbour_count )
        {
            
            strcpy(myID,strtok(file_line , " "));    
            strcpy(neighbour,strtok(NULL , " "));
            strcpy(cost,strtok(NULL , " "));
            strcpy(myIP,temp_str[atoi(myID)].ip);
            myport=temp_str[atoi(myID)].port;
            myid=atoi(myID);
            break;

            
            
        }


        count++;
        
        
    }
    fclose(file);
    file = fopen(file_name, "r");
    
    
    count=1;
    while(fgets(file_line,256,file)!=NULL)
    {
        if(count==1)
        {
            ID_count=atoi(file_line);
            
            
        }
        else if(count==2)
        {
            
            neighbour_count=atoi(file_line);
          
            
        }
        
        else if(count>2 && count < 3+ID_count )
        {
            
            strcpy(ID,strtok(file_line , " "));
            strcpy(IP,strtok(NULL , " "));
            strcpy(port,strtok(NULL , " "));
            struct in_addr net_IP;
            inet_aton(IP,&net_IP);
            int32_t temp_ip=net_IP.s_addr;
            if(strcmp(IP,myIP)==0 && myport==atoi(port))
            {
                node[1].id=atoi(ID);
                node[1].port=atoi(port);
                node[1].cost=0;
                node[1].isNeighbour=1;
                node[1].IP=temp_ip;
                node[1].updatecount=0;
                
                
            }
            
            else
            {
                node[j].id=atoi(ID);
                node[j].port=atoi(port);
                node[j].cost=MAX_INT;
                node[j].isNeighbour=0;
                node[j].IP=temp_ip;
                node[j].updatecount=0;
                j++;
                
                
            }
            
        }
        
        else if(count>= 3+ID_count && count < 3+ID_count+neighbour_count )
        {
            strcpy(myID,strtok(file_line , " "));
            strcpy(neighbour,strtok(NULL , " "));
            strcpy(cost,strtok(NULL , " "));
            myid=atoi(myID);
            int16_t neigh=(int16_t)atoi(neighbour);
            node[Index(neigh)].cost=(int16_t)atoi(cost);
            node[Index(neigh)].isNeighbour=1;

            
        }
        
        
        count++;
        
    }
    
    fclose(file);
 
    
   
    
    for (int i=1;i<=ID_count;i++)
    {
        rtable[i].id=0;
        rtable[i].cost=0;
        rtable[i].nexthop=0;
        
 
    }


    for (int i=1;i<=ID_count;i++)
    {
        
        rtable[i].id=node[i].id;
        if(i==1)
        {
            rtable[i].cost=0;
            rtable[i].nexthop=node[i].id;
            
        }
        
        else if(node[i].isNeighbour==1)
        {
            rtable[i].cost=node[i].cost;
            rtable[i].nexthop=node[i].id;
            
        }
        
        else
        {
            
            rtable[i].cost=(int16_t)MAX_INT;
            rtable[i].nexthop=-1;
            
        }
        
        
        
        
    }

   for(int i=1;i<=ID_count;i++)
   {
    
    for (int j=1;j<=ID_count;j++)
    {
      if(i==j)
      adjmatrix_neigh[i][j]=0;
      else
        adjmatrix_neigh[i][j]=MAX_INT;

    }
    
    }
    
    get_neighbours();

    

    //display_routing_table();
     //display_topology();
    
    
    printf("This Server has ID:%d\n",myid);
    printf("This Server is running at %s:%d\n",myIP,myport);


    struct sockaddr_in server_addr,client_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(myport);
    //fd_set master;
    int FD_max;
    FD_max=socket1;
    socket1=socket(AF_INET,SOCK_DGRAM,0);
    
    if(socket1<0)
    {
        perror("Server:Socket() init failed");
        return -1;
    }
    
    if(bind(socket1,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
    {
        perror("Bind() failed");
        close(socket1);
        return -1;
    }
    fd_set master_read,read_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&master_read);
    FD_SET(0,&master_read);
    FD_SET(socket1,&master_read);
    FD_max=socket1;
    struct timeval timer;
    timer.tv_sec=routing_time;
    timer.tv_usec=0;
    for(;;)
    {
    
        
        
        read_fds=master_read;
        
        int out = select(FD_max+1,&read_fds, NULL, NULL, &timer);
        
        if(out==0)
        {
                
        
            for(int i=2;i<=ID_count;i++)
            {

             node[i].updatecount++;


            }

            
            sendUpdate();    ////  Neighbour Update #################
            timer.tv_sec=routing_time;
            timer.tv_usec=0;
        
        }

       
        for(int i=0;i<=FD_max;i++)
        
        {
            if(FD_ISSET(i,&read_fds))
            {

                if(i==0)
              {
                char cmd_buf[50];
                int byte_count=read(0, cmd_buf , sizeof(cmd_buf));
            
                if ( byte_count >= 0 )
                {
                    cmd_buf[ byte_count-1] = '\0';
                }
                
                
                for(int i = 0;cmd_buf[i] != '\0'; i++)
                {
                    cmd_buf[i] = tolower(cmd_buf[i]);
                    
                }

                if(strstr(cmd_buf,"update")!=0)
                {   
                    if(strlen(cmd_buf)>=12)
                    {
                    char command[10];
                    char id1[4];
                    char id2[4];
                    char cost[5];

                    strcpy(command,strtok(cmd_buf , " "));
                    strcpy(id1,strtok(NULL , " "));
                    strcpy(id2,strtok(NULL , " "));
                    strcpy(cost,strtok(NULL , " "));
                    
                    if(strcmp(myID,id1)==0 && strcmp(cost,"inf")==0 )
                    {
                    
                        update_link(atoi(id2),MAX_INT);
                        node[Index(atoi(id2))].isNeighbour=0;
                        get_neighbours();
                    
                    }
                    else if(strcmp(myID,id1)==0 && strcmp(cost,"inf")!=0)
                    {
                    
                        update_link(atoi(id2),atoi(cost));
                    
                    
                    }
                    else

                    {
                        printf("%s\n","Please check update command" );
                    }
                    
                }
                else
                    printf("%s\n","Please Check format of Update Command");
            }
                else if(strcmp(cmd_buf,"step")==0)
                {
                    sendUpdate();
                    printf("%s\n","step SUCCESS" );
                }
                else if(strcmp(cmd_buf,"packets")==0)
                {
                    display_DV_count();
                    printf("%s\n","packets SUCCESS" );
                }
                else if(strcmp(cmd_buf,"display")==0)
                {
                    display_routing_table();
                    printf("%s\n","display SUCCESS" );
                }
                else if(strstr(cmd_buf,"disable")!=0)
                {   
                    char id3[4];
                    char command[10];

                    if(strlen(cmd_buf)>=9)
                    {
                    strcpy(command,strtok(cmd_buf , " "));
                    strcpy(id3,strtok(NULL , " "));
                    
                    //update_link(atoi(id3),MAX_INT);
                    node[Index(atoi(id3))].isNeighbour=0;
                    node[Index(atoi(id3))].cost=MAX_INT;

                    get_neighbours();
                    printf("%s\n","disable SUCCESS" );

                }
                else
                    printf("%s\n","Please Check the command disable");
                }
                
                else if(strcmp(cmd_buf,"crash")==0)
                {   

                    printf("%s\n","crash SUCCESS" );
                    FD_CLR(socket1,&master_read);
                    close(socket1);
                    exit(0);
                }
                
                else
                {
                
                    printf("Please Check Commands\n");
                
                
                }
                
            
               }
        
              else if(i==socket1)
            
             {
               socklen_t addrlen = sizeof(client_addr);
                char packet[100];
                
               recvfrom(socket1, packet,100, 0, (struct sockaddr *)&client_addr, &addrlen);
                packet_count++;
                deconcatinate_msg(packet);
            
             }

        }

        
        }        
            
        
        
        
    
    
    }

    

}
