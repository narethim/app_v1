/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//
// This is an illustration of how one could use virtualization techniques to
// allow running applications on virtual machines talking over simulated
// networks.
//
// The actual steps required to configure the virtual machines can be rather
// involved, so we don't go into that here.  Please have a look at one of
// our HOWTOs on the nsnam wiki for more details about how to get the 
// system confgured.  For an example, have a look at "HOWTO Use Linux 
// Containers to set up virtual networks" which uses this code as an 
// example.
//
// The configuration you are after is explained in great detail in the 
// HOWTO, but looks like the following:
//
//
// Allow ns-3 to ping a real host somewhere, using emulation mode
//
//   +----------------------+    
//   |          host        |
//   +----------------------+    
//   |    ns-3 simulation   |                                      
//   +----------------------+                  
//   |      ns-3 Node       |                 
//   |  +----------------+  |                 
//   |  |    ns-3 TCP    |  |              
//   |  +----------------+  |              
//   |  |    ns-3 IPv4   |  |                 
//   |  +----------------+  |                 
//   |  |   FdNetDevice  |  |                
//   |--+----------------+--+     
//   |       | eth0 |       |                
//   |       +------+       |    
//   |          |           |
//   +----------|-----------+ 
//              |
//              |         +---------+
//              .---------| GW host |--- (Internet) -----                             
//                        +---------+ 
//
//  1) You need to decide on a physical device on your real system, and either
//     overwrite the hard-configured device name below (eth0) or pass this
//     device name in as a command-line argument
//  2) The host device must be set to promiscuous mode
//     (e.g. "sudo ifconfig eth0 promisc")
//  3) Be aware that ns-3 will generate a fake mac address, and that in
//     some enterprise networks, this may be considered bad form to be
//     sending packets out of your device with "unauthorized" mac addresses
//  4) You will need to assign an IP address to the ns-3 simulation node that
//     is consistent with the subnet that is active on the host device's link.
//     That is, you will have to assign an IP address to the ns-3 node as if
//     it were on your real subnet.  Search for "Ipv4Address localIp" and
//     replace the string "1.2.3.4" with a valid IP address.
//  5) You will need to configure a default route in the ns-3 node to tell it
//     how to get off of your subnet. One thing you could do is a
//     'netstat -rn' command and find the IP address of the default gateway
//     on your host.  Search for "Ipv4Address gateway" and replace the string
//     "1.2.3.4" string with the gateway IP address.
//     If the --enable-sudo option was used to configure ns-3 with waf, then the following
//     step will not be necessary.
//
//     $ sudo chown root.root build/src/fd-net-device/ns3-dev-raw-sock-creator
//     $ sudo chmod 4755 build/src/fd-net-device/ns3-dev-raw-sock-creator
//

#include <string>
#include <iostream>
#include <fstream>

// https://www.nsnam.org/docs/models/html/fd-net-device.html
// https://www.nsnam.org/doxygen/fd-emu-ping_8cc_source.html
#include "ns3/fd-net-device-module.h"
#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
//#include "ipv4-l3-protocol.h"

// #include "ns3/netanim-module.h"
#include "ns3/applications-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TrafficReceiver");

static void
PingRtt (std::string context, Time rtt)
{
  NS_LOG_UNCOND ("Received Response with RTT = " << rtt);
}


int 
main (int argc, char *argv[])
{
    NS_LOG_INFO ("Jetson K3S Emulation");

    // Network IP is not fully describe but one forum found a solution
    // https://www.linuxquestions.org/questions/linux-newbie-8/address-overflow-913427/

    std::string deviceName1 ("enp0s8");
    std::string deviceIp1   ("10.161.29.20");
    std::string deviceMask1 ("255.255.255.0");

    std::string deviceName2 ("enp0s9");
    std::string deviceIp2   ("10.161.30.20");
    std::string deviceMask2 ("255.255.255.0");

    std::string deviceName3 ("enp0s10");
    std::string deviceIp3   ("10.161.31.20");
    std::string deviceMask3 ("255.255.255.0");

/*
    // 'netstat -rn' command and find the IP address of the default gateway on your host
    std::string deviceGateway1("0.0.0.0");
    std::string deviceGateway2("0.0.0.0");
*/
    std::string dataRate("5Mbps");
    std::string dataDelay("20ms");
    double stopTime = 30;

    //COMMAND LINE VARIABLES AND SETUP
    CommandLine cmd;

    // Add custom made command line flags
    cmd.AddValue("dataRate",  "Data Rate",    dataRate);
    cmd.AddValue("dataDelay", "Packet delay", dataDelay);
    cmd.AddValue("stopTime",  "Stop time (seconds)", stopTime);

    cmd.Parse (argc, argv);


    NS_LOG_INFO ("Start app...");

    std::cout << 
          "dataRate: "  << dataRate.c_str ()  <<
        ", dataDelay: " << dataDelay.c_str () <<
        ", stopTime: "  << stopTime           << std::endl;
    
//  LogComponentEnable ("TestApp", LOG_LEVEL_INFO);

    // Left Side
    Ipv4Address localIp1 (deviceIp1.c_str ());
    Ipv4Mask localMask1 (deviceMask1.c_str ());
    // Ipv4Address gateway1(deviceGateway1.c_str ());
    
    // Middle
    Ipv4Address localIp2 (deviceIp2.c_str ());
    Ipv4Mask localMask2 (deviceMask2.c_str ());
    // Ipv4Address gateway2(deviceGateway2.c_str ());

    // Right side 
    Ipv4Address localIp3 (deviceIp3.c_str ());
    Ipv4Mask localMask3 (deviceMask3.c_str ());
    // Ipv4Address gateway3(deviceGateway3.c_str ());

    //
    // We are interacting with the outside, real, world.  This means we have to 
    // interact in real-time and therefore means we have to use the real-time
    // simulator and take the time to calculate checksums.
    //
    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    //
    // Create 3 ghost nodes
    //
    NS_LOG_INFO ("  Create 3 Nodes");

    NodeContainer nodes;
    nodes.Create (3);

    //
    // Set up csma channel between ghost nodes
    //
    NS_LOG_INFO ("  Setup CSMA channel");

    CsmaHelper csma;

    csma.SetChannelAttribute ("DataRate", StringValue (dataRate));
    csma.SetChannelAttribute ("Delay",    StringValue (dataDelay));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (nodes);


    //
    // Add a default internet stack to the node.  This gets us the ns-3 versions
    // of ARP, IPv4, ICMP, UDP and TCP.
    //
    NS_LOG_INFO ("  Add Internet Stack");
    InternetStackHelper internetStackHelper;
    internetStackHelper.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase("192.134.135.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    //
    // Create an emu device, allocate a MAC address and point the device to the
    // Linux device name.  The device needs a transmit queueing discipline so
    // create a droptail queue and give it to the device.  Finally, "install"
    // the device into the node.
    //
    // Do understand that the ns-3 allocated MAC address will be sent out over
    // your network since the emu net device will spoof it.  By default, this
    // address will have an Organizationally Unique Identifier (OUI) of zero.
    // The Internet Assigned Number Authority IANA
    //
    //  http://www.iana.org/assignments/ethernet-numbers
    //
    // reports that this OUI is unassigned, and so should not conflict with
    // real hardware on your net.  It may raise all kinds of red flags in a
    // real environment to have packets from a device with an obviously bogus
    // OUI flying around.  Be aware.
    //
    NS_LOG_INFO ("  Create EmuFdNetDevice...");

    // emu1
    EmuFdNetDeviceHelper emu1;
    emu1.SetDeviceName (deviceName1);

    NetDeviceContainer devices1 = emu1.Install (nodes.Get (0));
    Ptr<NetDevice> device1 = devices1.Get (0);
    // device1->SetAttribute ("Address", Mac48AddressValue (Mac48Address::Allocate ()));

    // emu2
    EmuFdNetDeviceHelper emu2;
    emu2.SetDeviceName (deviceName2);

    NetDeviceContainer devices2 = emu2.Install (nodes.Get (1));
    Ptr<NetDevice> device2 = devices2.Get (0);
    // device2->SetAttribute ("Address", Mac48AddressValue (Mac48Address::Allocate ()));

    // emu3
    EmuFdNetDeviceHelper emu3;
    emu3.SetDeviceName (deviceName3);

    NetDeviceContainer devices3 = emu3.Install (nodes.Get (2));
    Ptr<NetDevice> device3 = devices3.Get (0);
    // device3->SetAttribute ("Address", Mac48AddressValue (Mac48Address::Allocate ()));



    NS_LOG_INFO ("  Create IPv4 Interfaces");

    Ptr<Ipv4> ipv4_1 = nodes.Get (0)->GetObject<Ipv4> ();
    uint32_t interface1 = ipv4_1->AddInterface (device1);
    Ipv4InterfaceAddress address1 = Ipv4InterfaceAddress (localIp1, localMask1);
    ipv4_1->AddAddress (interface1, address1);
    ipv4_1->SetMetric (interface1, 1);
    ipv4_1->SetUp (interface1);
    ipv4_1->SetAttribute("IpForward", BooleanValue(true));


    Ptr<Ipv4> ipv4_2 = nodes.Get (1)->GetObject<Ipv4> ();
    uint32_t interface2 = ipv4_2->AddInterface (device2);
    Ipv4InterfaceAddress address2 = Ipv4InterfaceAddress (localIp2, localMask2);
    ipv4_2->AddAddress (interface2, address2);
    ipv4_2->SetMetric (interface2, 1);
    ipv4_2->SetUp (interface2);
    ipv4_2->SetAttribute("IpForward", BooleanValue(true));


    Ptr<Ipv4> ipv4_3 = nodes.Get (2)->GetObject<Ipv4> ();
    uint32_t interface3 = ipv4_3->AddInterface (device3);
    Ipv4InterfaceAddress address3 = Ipv4InterfaceAddress (localIp3, localMask3);
    ipv4_3->AddAddress (interface3, address3);
    ipv4_3->SetMetric (interface3, 1);
    ipv4_3->SetUp (interface3);
    ipv4_3->SetAttribute("IpForward", BooleanValue(true));

//    device1->SetAttribute ("Address", Mac48AddressValue ("08:00:27:83:9c:c8"));
//    device2->SetAttribute ("Address", Mac48AddressValue ("08:00:27:a8:6e:a9"));

    device1->SetAttribute ("Address", Mac48AddressValue ("08:00:27:b3:a5:82"));   // enp0s8
    device2->SetAttribute ("Address", Mac48AddressValue ("08:00:27:7f:d9:0c"));   // enp0s9
    device3->SetAttribute ("Address", Mac48AddressValue ("08:00:27:dc:60:80"));   // enp0s10


    //
    // Create the ping application.  This application knows how to send
    // ICMP echo requests.  Setting up the packet sink manually is a bit
    // of a hassle and since there is no law that says we cannot mix the
    // helper API with the low level API, let's just use the helper.
    //

//    NS_LOG_INFO ("Create V4Ping Appliation");

    // std::string remoteIpstr("192.134.135.2");
//    std::string remoteIpstr("172.161.29.2");
//    Ipv4Address remoteIp(remoteIpstr.c_str());


    //
    // Print routing table to a file
    //
    // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Ipv4GlobalRoutingHelper g;
    
    g.PopulateRoutingTables ();
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routes_emu.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (15), routingStream);


    //
    // Hook a trace to print something when the response comes back.
    //
    Config::Connect ("/Names/app/Rtt", MakeCallback (&PingRtt));
 
    //
    // Enable a promiscuous pcap trace to see what is coming and going on our device.
    // To Check PCAP use the following:
    // tcpdump -nn -tt -r <PCAP FILE NAME>.pcap 

    emu1.EnablePcap ("fd-left",   device1, true);
    emu2.EnablePcap ("fd-middle", device2, true);
    emu2.EnablePcap ("fd-right",  device3, true);

    csma.EnablePcap ("csma-left",   csmaDevices.Get(0), true);
    csma.EnablePcap ("csma-middle", csmaDevices.Get(1), true);
    csma.EnablePcap ("csma-right",  csmaDevices.Get(2), true);


    //
    // Run the simulation for ten minutes to give the user time to play around
    //
    NS_LOG_INFO ("Run Emulation.");
    // Simulator::Stop (Seconds (25.0));
    Simulator::Stop (Seconds ( stopTime ));

    Simulator::Run ();

    // std::cout << "Animation Trace file created: " << animFile.c_str ()<<std::endl;
    Simulator::Destroy ();

    NS_LOG_INFO ("Done");
}

