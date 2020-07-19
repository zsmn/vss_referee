#include "vssreplacer.h"

QString VSSReplacer::name(){
    return "VSSReplacer";
}

VSSReplacer::VSSReplacer(const QString& refereeAddress, int replacerPort, const QString& firaSimAddress, int firaSimCommandPort)
{
    // Saving addresses
    _refereeAddress     = refereeAddress;
    _replacerPort       = replacerPort;
    _firaSimAddress     = firaSimAddress;
    _firaSimCommandPort = firaSimCommandPort;

    // Create a VSS Client to listen to replacement packets
    _vssClient = new VSSClient(_replacerPort, refereeAddress.toStdString());

    // Connect socket to send placement data to firaSim
    connect(firaSimAddress, firaSimCommandPort);

    // Reset vars
    _packetsReceived = 0;
    _blueSentPacket = false;
    _yellowSentPacket = false;
    _awaitingPackets = false;
}

VSSReplacer::~VSSReplacer(){
    delete _vssClient;
}

void VSSReplacer::initialization(){
    // Vision system connection (firaSim)
    if(_vssClient->open(true))
        std::cout << "[VSSReplacer] Listening to replace system on port " << _replacerPort << " and address = " << _refereeAddress.toStdString() << ".\n";
    else{
        std::cout << "[VSSReplacer] Cannot listen to replace system on port " << _replacerPort << " and address = " << _refereeAddress.toStdString() << ".\n";
        this->stopRunning();
    }
}

void VSSReplacer::loop(){
    VSSRef::team_to_ref::VSSRef_Placement packet;
    if(_vssClient->receive(packet)){
        if(packet.has_world()){
            VSSRef::Frame frame = packet.world();

            // Avoid to receive more than 1 packet from teams
            bool duplicated = false;
            if(frame.teamcolor() == VSSRef::Color::BLUE){
                if(!_blueSentPacket){
                    _mutex.lock();
                    _blueSentPacket = true;
                    _mutex.unlock();
                }
                else duplicated = true;
            }
            else if(frame.teamcolor() == VSSRef::Color::YELLOW){
                if(!_yellowSentPacket){
                    _mutex.lock();
                    _yellowSentPacket = true;
                    _mutex.unlock();
                }
                else duplicated = true;
            }

            if(!duplicated){
                // Save Frame from team
                frames[frame.teamcolor()] = frame;

                std::string teamColor = frame.teamcolor() == VSSRef::Color::BLUE ? "BLUE" : "YELLOW";
                std::cout << "[VSSReplacer] Received packet from team " << teamColor << std::endl;

                // Send signal to VSSReferee
                teamPlaced(frame.teamcolor());

                _packetsReceived++;
            }

            // If received packets from both teams
            if(_packetsReceived == 2){
                // reset
                _mutex.lock();
                _yellowSentPacket = false;
                _blueSentPacket   = false;
                _awaitingPackets  = false;
                _packetsReceived  = 0;
                _mutex.unlock();

                std::cout << "[VSSReplacer] Succesfuly received packets from the teams. Replacing now." << std::endl;

                /// TODO here
                /// Check if the positions are consistents to the foul (?)
                /// Place ball automatically too, based on the foul received.

                // Fill replacement packet with frame infos
                fillPacket(frames[VSSRef::Color::BLUE], frames[VSSRef::Color::YELLOW]);

                // Send packet to firaSim
                sendPacket(_replacementCommand);
            }
        }
    }
}

void VSSReplacer::finalization(){
    _vssClient->close();
}

void VSSReplacer::sendPacket(fira_message::sim_to_ref::Replacement command){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), msg.length()) == -1){
        std::cout << "[VSSReplacer] Failed to write to firaSim socket: " << _socket.errorString().toStdString() << std::endl;
    }
}

bool VSSReplacer::connect(const QString &firaSimAddress, int firaSimCommandPort){
    // Connect to referee address and port
    if(_socket.isOpen())
        _socket.close();

    _socket.connectToHost(firaSimAddress, firaSimCommandPort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

    std::cout << "[VSSReplacer] Writing to firaSim replacement system on port " << firaSimCommandPort << " and address = " << firaSimAddress.toStdString() << ".\n";

    return true;
}

void VSSReplacer::disconnect() {
    // Close referee socket
    if(_socket.isOpen()){
        _socket.close();
    }
}

bool VSSReplacer::isConnected() const {
    return (_socket.isOpen());
}

QString VSSReplacer::getFoulNameById(VSSRef::Foul foul){
    switch(foul){
        case VSSRef::Foul::FREE_BALL:    return "FREE_BALL";
        case VSSRef::Foul::FREE_KICK:    return "FREE_KICK";
        case VSSRef::Foul::GOAL_KICK:    return "GOAL_KICK";
        case VSSRef::Foul::PENALTY_KICK: return "PENALTY_KICK";
    }

    return "FOUL NOT IDENTIFIED";
}

void VSSReplacer::takeFoul(VSSRef::Foul foul){
    std::cout << "[VSSReplacer] Command from ref: " << getFoulNameById(foul).toStdString() << ". Awaiting teams replacement packets." << std::endl;

    _mutex.lock();
    _yellowSentPacket = false;
    _blueSentPacket   = false;
    _awaitingPackets  = true;
    _packetsReceived  = 0;
    _timer.start();
    _mutex.unlock();
}

void VSSReplacer::fillPacket(VSSRef::Frame frameBlue, VSSRef::Frame frameYellow){
    // Filling blue robots
    int sz = frameBlue.robots_size();
    for(int x = 0; x < sz; x++){
        // Taking robot from frame
        VSSRef::Robot robotAt = frameBlue.robots(x);
        parseRobot(&robotAt, VSSRef::Color::BLUE);
    }

    // Filling yellow robots
    sz = frameYellow.robots_size();
    for(int x = 0; x < sz; x++){
        // Taking robot from frame
        VSSRef::Robot robotAt = frameBlue.robots(x);
        parseRobot(&robotAt, VSSRef::Color::YELLOW);
    }
}

void VSSReplacer::parseRobot(VSSRef::Robot *robot, VSSRef::Color robotTeam){
    // Creating firaRobot and robotPosition
    fira_message::sim_to_ref::RobotReplacement *firaRobot = _replacementCommand.add_robots();
    fira_message::Robot *robotPosition = new fira_message::Robot();

    // Parsing position
    robotPosition->set_x(robot->x());
    robotPosition->set_y(robot->y());
    robotPosition->set_robot_id(robot->robot_id());
    robotPosition->set_orientation(robot->orientation());

    // Inserting infos in firaRobot
    firaRobot->set_turnon(true);
    firaRobot->set_yellowteam(robotTeam);
    firaRobot->set_allocated_position(robotPosition);
}
