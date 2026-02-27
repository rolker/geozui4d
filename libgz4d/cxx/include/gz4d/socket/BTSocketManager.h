#ifndef GZ4D_BTSOCKETMANAGER_H
#define GZ4D_BTSOCKETMANAGER_H

#include <gz4d/gz4d.h>
#include <gz4d/socket/BTHCISocket.h>

#ifndef WIN32
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#endif


#include <vector>
#include <string>

namespace gz4d
{
    struct BTDevice
    {
        std::string name;
        std::string address;
        BTDevice(std::string const &name, std::string const &address)
            :name(name),address(address){}
    };
    class BTSocketManager //: public SocketManager
    {
        public:
            inline std::vector<BTDevice> Scan(unsigned char c0=0, unsigned char c1=0, unsigned char c2=0, bool restrictClass = false)
            {
                std::vector<BTDevice> ret;
                #ifndef WIN32
                char addr[19] = { 0 };
                char name[248] = { 0 };
                
                int dev_id = hci_get_route(NULL);
                BTHCISocket sock;
                sock.Bind(BTHCISocket::Address(dev_id));
                
                inquiry_info *ii = (inquiry_info*)malloc(255*sizeof(inquiry_info));
                int num_rsp = hci_inquiry(dev_id, 1, 255, NULL, &ii, IREQ_CACHE_FLUSH);
                for (int i = 0; i < num_rsp; i++)
                {
                    if(!restrictClass || (ii[i].dev_class[0]==c0 && ii[i].dev_class[1]==c1 && ii[i].dev_class[2]==c2))
                    {
                        ba2str(&ii[i].bdaddr, addr);
                        //std::cerr << addr << ": " << (int)ii[i].dev_class[0] << "," << (int)ii[i].dev_class[1] << "," << (int)ii[i].dev_class[2] << "\n";
                        memset(name, 0, sizeof(name));
                        if(!restrictClass)
                            hci_read_remote_name(sock, &ii[i].bdaddr, sizeof(name), name, 0);
                        ret.push_back(BTDevice(name,addr));
                    }
                }
                free(ii);
                #endif
                return ret;
            }
            
    };
}

#endif
