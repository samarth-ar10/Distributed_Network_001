Distributed_Network_00


ABSTRACT


The purpose of this project is to process sets of data over a Decentralized Network architecture. The data included could be any form which is acceptable and could be processed by a processing unit / processing node in the network. This would provide flexibility in developing scalable robotic and IOT technologies where the available resources, ranging from processing power to processing time, tends to be variable, thus scaling the resources for the overall system improvement.


Defined in the project are processing nodes will be user defined services which would register themselves as features over the network to process the closest data node (processing unit which is generating the data) when it is requested. This would facilitate a more dynamic environment to increase or decrease the processing nodes as per user’s requirements. It would also tend to improve the performance of the nodes on the network by selecting the node closest without having a central reference point to check the availability every time. Such an architecture is intended to boost Robotics to manage delays caused by various sensors, actuators and other factors which results in wastage of resources especially in a setup similar to swarm. 


METHODOLOGY


Laying down the important rules that needs to be adhered to while creating this architecture. 
* The node should be able to ask for service at any point in time regardless of its availability.
* Looping of a request between the same nodes while traversing through the network should be minimalized.
* If a request has been accepted, a procedure should be in place to notify other nodes so as to prevent them from offering any further assistance.


Basic Architecture


The Processing node( a single  service providing unit on a network) is divided into two modules, Algorithm node and Networking node. The Networking node is further divided into two sub modules, Outer Networking module and Inner Networking Module. The Outer Networking module is the actual gateway for the system and the network. This division is based on the manner of implementation in the case where all the three modules are run parallelly.
The flow involves the Algorithm module to ready up the shared memory, pass on the control to the user program while parallelly initiating the Inner and the Outer Networking module and passing the controls to them. Fig 3.1 illustrates the above.
Fig. Basic flow 
  

















User Program


The user program is the section of the code which is intended to be run by the user on the basis of providing or using services. This piece of the architecture is made to be the simplest of all the modules to facilitate user acceptance. Almost all the background features are hidden away from the user.


The basic flow of the user program would involve initializing a packet to collect all the data required by the user to be processed. After the collection of the data it is encapsulated by some details pertaining to the data which is not shown to the user. After the initialization of the packet the control is passed by the user to send the packet to the Inner Networking module which would relay it further.


Fig. Control flow of the user program
  





















Algorithm Module


Fig. flow of Algorithm Module
  

* * * * * * 





















* * Parameter initialization 
   * In this part of the flow the data is picked from the command like or the user is asked for the details.The details included are:
      * A name for the node
      * the command required to run the user’s programming
      * a small description
* Initializing a log file
   * This file would be unique and would house all the incoming and outgoing packets to the node.
* Initializing the shared Memory
   * This section is to make sure that all the parameters of the shared memory are properly initialized with adequate memory and a unique number to access that memory is obtained which is further passed on to the rest of the nodes to access it.
* Initializing rest of the modules
   * A fork() is used to create a child process which houses the rest of the modules. It is important to note that the user program and the Outer Networking Module are housed in separate child processes but the Inner Networking Module is not called separately. Instead the Algorithm module monitors the flag inside the shared memory and calls all the appropriate functions in the Inner Networking Module.
* It sits in a constant looping state monitoring the changes in the flags of the shared memory.