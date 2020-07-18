#include "vssreplacer.h"

QString VSSReplacer::name(){
    return "VSSReplacer";
}

VSSReplacer::VSSReplacer(const std::string& refereeAddress, int replacerPort)
{
    _refereeAddress = refereeAddress;
    _replacerPort = replacerPort;
    // Create a VSS Client to listen to replacement packets
    _vssClient = new VSSClient(_replacerPort, refereeAddress);
}

void VSSReplacer::initialization(){
    // Vision system connection (firaSim)
    if(_vssClient->open(true))
        std::cout << "[VSSReplacer] Listening to replace system on port " << _replacerPort << " and address = " << _refereeAddress << ".\n";
    else{
        std::cout << "[VSSReplacer] Cannot listen to replace system on port " << _replacerPort << " and address = " << _refereeAddress << ".\n";
        this->stopRunning();
    }
}

void VSSReplacer::loop(){
    VSSRef::team_to_ref::VSSRef_Placement packet;

    // debugging
    if(_vssClient->receive(packet)){
        if(packet.has_world()){
            VSSRef::Frame frame = packet.world();
            std::cout << "Frame received: " << std::endl;
            int robots_sz = frame.robots_size();
            std::cout << "TEAM: " << frame.teamcolor() << std::endl;
            for(int x = 0; x < robots_sz; x++){
                std::cout << "Robot " << x << " x: " << frame.robots(x).x() << " y: " << frame.robots(x).y() <<  " orientation: " << frame.robots(x).orientation() << std::endl;
            }

            /// TODO here
            /// Send placement commands to FIRASim
            /// Check if the positions are consistents to the foul (?)
        }
    }
}

void VSSReplacer::finalization(){
    _vssClient->close();
}
