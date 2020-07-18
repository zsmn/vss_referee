#include "vssreferee.h"

QString VSSReferee::name(){
    return "VSSReferee";
}

VSSReferee::VSSReferee(VSSVisionClient *visionClient, const QString& refereeAddress, int refereePort)
{
    _visionClient = visionClient;
    _refereeAddress = refereeAddress;
    _refereePort = refereePort;

    connect(refereeAddress, refereePort);
}

VSSReferee::~VSSReferee(){

}

void VSSReferee::initialization(){
}

void VSSReferee::loop(){
    /// TODO HERE
    /// Receive and process VSSVisionClient informations to check fouls
    _refereeCommand.set_foul(VSSRef::Foul::FREE_BALL);
    _refereeCommand.set_teamcolor(VSSRef::Color::BLUE);

    if(isConnected()){
        sendPacket(_refereeCommand);
    }
}

void VSSReferee::finalization(){
    disconnect();
    std::cout << "[VSSReferee] Thread ended" << std::endl;
}

void VSSReferee::sendPacket(VSSRef::ref_to_team::VSSRef_Command command){
    std::string msg;
    command.SerializeToString(&msg);

    if(_socket.write(msg.c_str(), msg.length()) == -1){
        std::cout << "[VSSReferee] Failed to write to socket: " << _socket.errorString().toStdString() << std::endl;
    }
}

bool VSSReferee::connect(const QString &refereeAddress, int refereePort){
    // Connect to referee address and port
    if(_socket.isOpen())
        _socket.close();

    _socket.connectToHost(refereeAddress, refereePort, QIODevice::WriteOnly, QAbstractSocket::IPv4Protocol);

    std::cout << "[VSSReferee] Writing to referee system on port " << _refereePort << " and address = " << _refereeAddress.toStdString() << ".\n";

    return true;
}

void VSSReferee::disconnect() {
    // Close referee socket
    if(_socket.isOpen()){
        _socket.close();
    }
}

bool VSSReferee::isConnected() const {
    return (_socket.isOpen());
}


