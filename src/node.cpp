
/*!
 * \file node.cpp
 *
 * \brief This class defines the attributes and methods represented by each node.
 * 	  Each node can either be a server or a client or both a server and a client 
 * 	  Depending on the scenario this class will be inherited by Server or Client class 
 *
 * \author Suraj Chafle <schafle@hawk.edu>
 * 
 * \date Tue Jun 21 18:27:43 CDT 2016
 */

#include "node.h"

/* Constructor */
Node::Node(std::string hostName, int portNumber, int PosNum, string filename) {
	hostName = hostName;
	portNumber = portNumber;
	available_nodes = get_list_of_all_nodes(filename);
	position_in_cluster = PosNum;
}

/* 
 * This function returns list of all nodes available in the cluster
 * Should be called only once
 */

std::vector<std::string> Node::get_list_of_all_nodes(std::string filename){
	std::vector<std::string> nodes;
	std::string line;
	ifstream myfile (filename.c_str());
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			nodes.push_back(line);
		}
		myfile.close();
	}

	else cout << "Unable to open file";

	//std::vector<std::string> IPs{"ip0", "localhost", "ip2", "ip3", "ip4", "ip5", "ip6", "ip7"}; 
	return nodes;
}


/* Here goes the logic for get parent method */
std::string Node::get_parent(int initialNodeIndex, int numOfBranches){
	int curNodeIndex = position_in_cluster;
	std::vector<std::string> IPs = available_nodes;
	int adjustedIndex=(curNodeIndex-initialNodeIndex+IPs.size())%IPs.size();
	std::string parentIP = IPs[(adjustedIndex-1)/numOfBranches];
	return parentIP;

}

/* Here goes the logic for get children method */
std::vector<std::string> Node::get_children(int starting_node, int num_of_branches){

	int pos_of_current_node = position_in_cluster;
	std::vector<std::string> IPs = available_nodes;
	std::vector<std::string> children_IPs;
	int adjustedIndex=(pos_of_current_node - starting_node + IPs.size())%IPs.size();
	for(int i=0;i<num_of_branches;i++){
		int childIndex= num_of_branches*adjustedIndex+1+i;
		std::string child=IPs[childIndex];
		if(childIndex<IPs.size()){
			children_IPs.push_back(child);
		}
		else{
			break;
		}
	}
	return children_IPs;
}

bool Node::am_i_leaf(int starting_node, int num_of_branches){

	int pos_of_current_node = position_in_cluster;
	std::vector<std::string> IPs = available_nodes;
	int adjustedIndex=(pos_of_current_node - starting_node + IPs.size())%IPs.size();
	if(adjustedIndex*num_of_branches+1>IPs.size()-1){
		return true;
	}
	return false;
}

/* Get message from Parent */
std::string Node::get_message_from_parent(std::string ParentHostName, int PortNumber){
	TCPStream* stream = NULL;
	TCPAcceptor* acceptor = NULL;
	acceptor = new TCPAcceptor( 8012, ParentHostName.c_str());
	char line[256];
	if (acceptor->start() == 0) {
		stream = acceptor->accept();
		if (stream != NULL) {
			ssize_t len;
			while ((len = stream->receive(line, sizeof(line))) > 0) {
				line[len] = 0;
			}
			std::cout << "Got the message from parent: " <<line << std::endl;
		}
	}

	delete stream;
	return 0;	
}

/* Send message to Children */
int Node::send_message_to_children(std::string childHostName, int PortNumber, std::string message){
	/* Establish the connection */
	std::cout << "Sending query to "<< childHostName << std::endl;
	TCPConnector* connector = new TCPConnector();
	TCPStream* stream = connector->connect(childHostName.c_str(),PortNumber);
	if (stream) {
		stream->send(message.c_str(), message.size());
		std::cout << "sent the query to " << childHostName << std::endl;
		return 1;
	}

	delete stream;
	return 0;
}

/* Wait for children to send message */
std::string Node::receive_message_from_children(std::string childHostName, int PortNumber){
	TCPStream* stream = NULL;
	TCPAcceptor* acceptor = NULL;
	acceptor = new TCPAcceptor( 8013, childHostName.c_str());
	char line[1000];
	if (acceptor->start() == 0) {
		stream = acceptor->accept();
		if (stream != NULL) {
			ssize_t len;
			while ((len = stream->receive(line, sizeof(line))) > 0) {
				line[len] = 0;
			}
		}
	}
	delete stream;
	return line;
}

/* Send message to parent */
int Node::send_message_to_parent(std::string ParentHostName, int PortNumber, std::string message){
	/* Establish the connection */
	std::cout << "Sending answer to "<< ParentHostName << std::endl;
	TCPConnector* connector = new TCPConnector();
	//TCPStream* stream = connector->connect(ParentHostName.c_str(),PortNumber);
	TCPStream* stream = connector->connect(ParentHostName.c_str(), 8013);
	if (stream) {
		stream->send(message.c_str(), message.size());
		std::cout << "sent the reply to " << ParentHostName << std::endl;
		return 1;
	}
	delete stream;
	return 0;
}
