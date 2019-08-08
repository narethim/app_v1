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

 #include "ns3/netanim-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/flow-monitor-module.h"


using namespace ns3;

// To enable it: export NS_LOG=TrafficReceiver=info
// To disable it: export NS_LOG=
//
NS_LOG_COMPONENT_DEFINE ("TrafficReceiver");

static void
PingRtt (std::string context, Time rtt)
{
  NS_LOG_UNCOND ("Received Response with RTT = " << rtt);
}


int 
main (int argc, char *argv[])
{
    std::string data1Rate("10Mbps");
    std::string data1Delay("350ms");
    std::string data2Rate("10Mbps");
    std::string data2Delay("150ms");
    double stopTime = 30;
    CommandLine cmd;

    cmd.AddValue("data1Rate",  "Point-toPoint link 1 Data Rate",    data1Rate);
    cmd.AddValue("data1Delay", "Point-toPoint link 1 Packet delay", data1Delay);

    cmd.AddValue("data2Rate",  "Point-toPoint link 2 Data Rate",    data2Rate);
    cmd.AddValue("data2Delay", "Point-toPoint link 2 Packet delay", data2Delay);

    cmd.AddValue("stopTime",  "Stop time (seconds)", stopTime);

    cmd.Parse (argc, argv);

    NS_LOG_INFO ("Start app...");

    std::cout << argv[0] << ":\n\t" <<
         " data1Rate: "  << data1Rate.c_str ()  <<
        ", data1Delay: " << data1Delay.c_str () <<
        ", data2Rate: "  << data2Rate.c_str ()  <<
        ", data2Delay: " << data2Delay.c_str () <<
        ", stopTime: "   << stopTime            << std::endl;

    NS_LOG_INFO ("Jetson K3S Emulation");

    // Network IP is not fully describe but one forum found a solution
    // https://www.linuxquestions.org/questions/linux-newbie-8/address-overflow-913427/
    // Ipv4Address gateway3(deviceGateway3.c_str ());


    //
    // We are interacting with the outside, real, world.  This means we have to 
    // interact in real-time and therefore means we have to use the real-time
    // simulator and take the time to calculate checksums.
    //
    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    //
    // Create ghost nodes
    // see nsnam.org/docs/tutorial/html/building-topologies.html
    //
    NS_LOG_INFO ("Create Point-to-Point1 Nodes");
    NodeContainer ptop1Nodes;
    ptop1Nodes.Create (2);

    NS_LOG_INFO ("Create Point-to-Point2 Nodes");
    NodeContainer ptop2Nodes;
    ptop2Nodes.Add(ptop1Nodes.Get(1));
    ptop2Nodes.Create (1);

    //
    // Set up point-to-point channel between ghost nodes
    //

    PointToPointHelper ptop1;
    ptop1.SetDeviceAttribute ("DataRate", StringValue (data1Rate));
    ptop1.SetChannelAttribute ("Delay", StringValue (data1Delay));

    NetDeviceContainer ptop1Devices;
    ptop1Devices = ptop1.Install (ptop1Nodes);


    PointToPointHelper ptop2;
    ptop2.SetDeviceAttribute ("DataRate", StringValue (data2Rate));
    ptop2.SetChannelAttribute ("Delay", StringValue (data2Delay));
    NetDeviceContainer ptop2Devices;
    ptop2Devices = ptop2.Install (ptop2Nodes);

    //
    // Add a default internet stack to the node.  This gets us the ns-3 versions
    // of ARP, IPv4, ICMP, UDP and TCP.
    //
    NS_LOG_INFO ("Add Internet Stack");
    InternetStackHelper stack;
    stack.Install (ptop1Nodes.Get(0));
    stack.Install(ptop2Nodes);

    Ipv4AddressHelper address;
    address.SetBase("192.134.135.0", "255.255.255.0");
    Ipv4InterfaceContainer ptop1Interfaces;
    ptop1Interfaces = address.Assign(ptop1Devices);

    address.SetBase("198.134.135.0", "255.255.255.0");
    Ipv4InterfaceContainer ptop2Interfaces;
    ptop2Interfaces = address.Assign(ptop2Devices);

    // Create an emu devices, allocate a MAC address and point the device to the
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

    NS_LOG_INFO ("Create Emu Devices");

    std::string deviceName1 ("enp0s8");
    std::string deviceIp1   ("10.161.29.20");
    std::string deviceMask1 ("255.255.255.0");

    std::string deviceName2 ("enp0s9");
    std::string deviceIp2   ("10.161.30.20");
    std::string deviceMask2 ("255.255.255.0");

    std::string deviceName3 ("enp0s10");    
    std::string deviceIp3   ("10.161.31.20");
    std::string deviceMask3 ("255.255.255.0");

    // Left Side
    Ipv4Address localIp1 (deviceIp1.c_str ());
    Ipv4Mask localMask1 (deviceMask1.c_str ());
    EmuFdNetDeviceHelper emu1;
    emu1.SetDeviceName (deviceName1);
    NetDeviceContainer devices1 = emu1.Install (ptop1Nodes.Get (0));
    Ptr<NetDevice> device1 = devices1.Get (0);
    
    // Middle
    Ipv4Address localIp2 (deviceIp2.c_str ());
    Ipv4Mask localMask2 (deviceMask2.c_str ());
    EmuFdNetDeviceHelper emu2;
    emu2.SetDeviceName (deviceName2);
    NetDeviceContainer devices2 = emu2.Install (ptop1Nodes.Get (1));
    Ptr<NetDevice> device2 = devices2.Get (0);

    // Right
    Ipv4Address localIp3 (deviceIp3.c_str ());
    Ipv4Mask localMask3 (deviceMask3.c_str ());
    EmuFdNetDeviceHelper emu3;
    emu3.SetDeviceName (deviceName3);
    NetDeviceContainer devices3 = emu3.Install (ptop2Nodes.Get (1));
    Ptr<NetDevice> device3 = devices3.Get (0);

    NS_LOG_INFO ("Create IPv4 Interface");
    Ptr<Ipv4> ipv4_1 = ptop1Nodes.Get (0)->GetObject<Ipv4> ();
    uint32_t interface1 = ipv4_1->AddInterface (device1);
    Ipv4InterfaceAddress address1 = Ipv4InterfaceAddress (localIp1, localMask1);
    ipv4_1->AddAddress (interface1, address1);
    ipv4_1->SetMetric (interface1, 1);
    ipv4_1->SetUp (interface1);
    ipv4_1->SetAttribute("IpForward", BooleanValue(true));

    Ptr<Ipv4> ipv4_2 = ptop2Nodes.Get (0)->GetObject<Ipv4> ();
    uint32_t interface2 = ipv4_2->AddInterface (device2);
    Ipv4InterfaceAddress address2 = Ipv4InterfaceAddress (localIp2, localMask2);
    ipv4_2->AddAddress (interface2, address2);
    ipv4_2->SetMetric (interface2, 1);
    ipv4_2->SetUp (interface2);
    ipv4_2->SetAttribute("IpForward", BooleanValue(true));

    Ptr<Ipv4> ipv4_3 = ptop2Nodes.Get (1)->GetObject<Ipv4> ();
    uint32_t interface3 = ipv4_3->AddInterface (device3);
    Ipv4InterfaceAddress address3 = Ipv4InterfaceAddress (localIp3, localMask3);
    ipv4_3->AddAddress (interface3, address3);
    ipv4_3->SetMetric (interface3, 1);
    ipv4_3->SetUp (interface3);
    ipv4_3->SetAttribute("IpForward", BooleanValue(true));

    device1->SetAttribute ("Address", Mac48AddressValue ("08:00:27:b3:a5:82"));   // enp0s8
    device2->SetAttribute ("Address", Mac48AddressValue ("08:00:27:7f:d9:0c"));   // enp0s9
    device3->SetAttribute ("Address", Mac48AddressValue ("08:00:27:dc:60:80"));   // enp0s10

    //

    //
    // Create the ping application.  This application knows how to send
    // ICMP echo requests.  Setting up the packet sink manually is a bit
    // of a hassle and since there is no law that says we cannot mix the
    // helper API with the low level API, let's just use the helper.
    //
    NS_LOG_INFO ("Create V4Ping Application");
    std::string remoteIpstr("10.161.30.30");
    Ipv4Address remoteIp(remoteIpstr.c_str());


    //
    // Create the animation object and configure for specified output
    //
    std::string animFile = "emu-animation.xml" ;  // Name of file for animation output
//  AnimationInterface anim (animFile);
//  anim.EnablePacketMetadata (); // Optional
//  anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (25)); // Optional


    //
    // Print routing table to a file
    //
    //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Ipv4GlobalRoutingHelper g;
    g.PopulateRoutingTables ();

    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routes_emu.routes", std::ios::out);
    g.PrintRoutingTableAllAt (Seconds (1), routingStream);


    //
    // Hook a trace to print something when the response comes back.
    //
    Config::Connect ("/Names/app/Rtt", MakeCallback (&PingRtt));
 
    //
    // Enable a promiscuous pcap trace to see what is coming and going on our device.
    // To Check PCAP use the following:
    // tcpdump -nn -tt -r <PCAP FILE NAME>.pcap 

//    emu1.EnablePcap ("fd-left", device1, true);
//    emu2.EnablePcap ("fd-right", device2, true);
//    pointToPoint.EnablePcap ("p2p-left", p2pDevices.Get(0), true); 
//    pointToPoint.EnablePcap ("p2p-right", p2pDevices.Get(1), true);
//    ptop1.EnablePcap ("ptop1-left", ptop1Devices.Get(0), true);
//    ptop2.EnablePcap ("ptop2-right", ptop2Devices.Get(1), true);
    ptop1.EnablePcapAll ("ptop1");
    ptop2.EnablePcapAll ("ptop2");

    //
    // Run the simulation for ten minutes to give the user time to play around
    //
    NS_LOG_INFO ("Run Emulation.");

//    Simulator::Stop (Seconds (100000.0));
    Simulator::Stop (Seconds (stopTime));

    Simulator::Run ();

    // std::cout << "Animation Trace file created: " << animFile.c_str ()<<std::endl;
        
    //Set up flow monitor parameters
    FlowMonitorHelper flowmon_help;
    Ptr<FlowMonitor> flowmon = flowmon_help.InstallAll();
    flowmon->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    flowmon->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    flowmon->SetAttribute("PacketSizeBinWidth", DoubleValue(20));
    flowmon->CheckForLostPackets();
    flowmon->SerializeToXmlFile("flowmon-results.xml", true, true);     

    Simulator::Destroy ();
    NS_LOG_INFO ("Done");
}

