//********************************************************************
// Mehmet Ozkan
// Advanced Operating Systems
// Project #4: Suzuki-Kazami Algoritm
// December 7, 2018
// Instructor: Dr. Belkhouche
//********************************************************************


#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>  



#define TOKENTAG 99
#define REQUESTTAG 88
#define STOP 999
int main(int argc, char *argv[])
{
	typedef enum { false, true } bool;
	int root = 0, rank, requested_rank,size;
	int  message_flag, token_owner, owner_flag, owner,counter=0;
	int sim_time, sim_finaltime, max_wait, wait_finaltime, wait_finaltime2,cs_endTime, waiting_time,waiting_time2;
	
	double t1, t2;
	time_t t;
	int inside;
	bool sim_flag, flag_waiting,flag_waiting2;
	int REQ_INFO;//SN declaration
	int FINALIZE;
	
struct //Define states 
{
   bool RELEASED;
   bool GRANTED;
   bool REQUESTED;
 
}procs_state;

procs_state.RELEASED=true; // initialize the procs state as RELEASED



	MPI_Status status;
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &size); 
	
	int RN[size];//define RN array 
	
struct //Define token structure
{
  int Q[100];
  int LN[size];
     
}token;



int blocklengths[2]={100,size};

MPI_Datatype types[2]={MPI_INT,MPI_INT};
MPI_Aint displacements[2];
MPI_Datatype ptype;
MPI_Aint intex;
MPI_Type_extent(MPI_INT, &intex);
displacements[0] = (MPI_Aint) 0; 
displacements[1] = intex * (100);
MPI_Type_struct(2, blocklengths, displacements, types, &ptype);
MPI_Type_commit(&ptype);
MPI_Request request;

if(argc !=3) // error checking if the  number of arguments are different than 3 
	{
		printf("[Error]:: Process[%d] could not start! Please type the correct number of arguments!\n", rank);
		MPI_Finalize();
		return 0;
	}
	
		
	if(size < 4) // error checking if the number of processes are at least 4

	{
		
		printf("[Error]:: Number of process can not be less than 4\n");                       
		MPI_Finalize();
		return 0;
		
	}	
	if(size > 8) // error checking if the number of processes are larger than 8

	{
		
		printf("[Error]:: Number of process can not be more than 8\n");                       
		MPI_Finalize();
		return 0;
		
	}	
	//initialize LN and RN
	for(int i=0; i<size; i++)
	{
		token.LN[i] = 0;
		RN[i] = 0;
	}
	
		
	srand((unsigned) time(&t));//define the time seed 

	sscanf(argv[1], "%d", &sim_time); //get the sim time 
	sscanf(argv[2], "%d", &max_wait); // get the max waiting time 
	
	if(max_wait>=sim_time) // error checking if the simulation time is less than 10

	{
		
		printf("[Error]:: Waiting time can not be more than simulation time\n");                       
		MPI_Finalize();
		return 0;
		
	}	
	
	if(sim_time < 10) // error checking if the simulation time is less than 10

	{
		
		printf("[Error]:: Simulation time can not be less than 10\n");                       
		MPI_Finalize();
		return 0;
		
	}	
	
	if(max_wait>5) // error checking if the waiting time is more than 5

	{
		
		printf("[Error]:: Waiting time can not be more than 5 seconds\n");                       
		MPI_Finalize();
		return 0;
		
	}
	
	
	
	if(rank == 0)
	{
	
		token_owner = rand() % size; // assign the token randomly to the any process 
			
		for(int i=0;i<size;i++)
		{
			if(i==0)
			{
				if(i == token_owner)// if the token owner is root 
				 {
				owner_flag=1;
				printf("Process %d: has the token at first\n", i);
				 }
			}
			else if(i!= 0)
			{
			 if(i == token_owner) // if the token owner is other than root 
			 {
			 owner = 1;
			 printf("Process %d: has the token at first\n", i);
			 }
			 else
			 {					 
			 owner = 0;
			 }
			 MPI_Send(&owner, 1, MPI_INT, i,TOKENTAG, MPI_COMM_WORLD); // notify other process that who has a token first 
			}
					
		}
	}
	else
	{
		MPI_Recv(&owner_flag, 1, MPI_INT, 0, TOKENTAG, MPI_COMM_WORLD, &status); // receive the token owner info from root process 
		
	}
	
	if(rank == token_owner) //  if the current rank is token_owner , directly go to the state of GRANTED
	{
	procs_state.GRANTED==true;	
	}
	
	
sim_flag=true; // assign the simulation flag as true firstly 

sim_finaltime = MPI_Wtime() + sim_time;// assign the sim_finaltime
MPI_Barrier(MPI_COMM_WORLD);

while(sim_flag)// while the sim_flag is true 
	{
		if(procs_state.RELEASED==true) // if the procs state is RELEASED
		{
			
			flag_waiting = true; //assign waiting flag  as true
			waiting_time = rand() % (max_wait+1); // generate waiting time between 0 and 5
			wait_finaltime = MPI_Wtime() + waiting_time;//assign the waiting time
			
			while(flag_waiting) //while the flag_waiting is true 
			{
				
				
				MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&message_flag,&status); // check if there is any message available 
				usleep(2);// sleep 2 microseconds to check whether or not any message is available
				
					if(message_flag) //if there is any message 
					{
						if( status.MPI_TAG == REQUESTTAG)// if the message is coming with REQUESTTAG
						{
						MPI_Recv(&REQ_INFO, 1, MPI_INT, MPI_ANY_SOURCE, REQUESTTAG, MPI_COMM_WORLD, &status); 
						
						if(REQ_INFO> RN[status.MPI_SOURCE])//if the SN of the requested process is larger than RN
						{
							RN[status.MPI_SOURCE] = REQ_INFO; //update RN of the requested process with its SN 
						}
						
						printf("Rank %d received critical section request from rank %d\n", rank, status.MPI_SOURCE); // display critical section request 
						if( RN[status.MPI_SOURCE] == token.LN[status.MPI_SOURCE]+1 && owner_flag ==1) // if the request of the process RN is equal to LN of the current process +1
						{																			 // and if the token_owner still have the token
						
							printf("Rank %d is sending the token to rank %d\n", rank, status.MPI_SOURCE);
							MPI_Send(&token, 1, ptype, status.MPI_SOURCE, TOKENTAG, MPI_COMM_WORLD);// send the token to the requested process 
							owner_flag = 0; // assign owner_flag as 0 after owner is released the its token
						}
						}
					}
				if( MPI_Wtime() >= wait_finaltime)// if the current time is larger than simulation time , stop the simulation 
					flag_waiting = false; 
			}
			
			
			
			if( MPI_Wtime() < sim_finaltime )
			{
				
				if(owner_flag==1) //if the current process has token, go to GRANTED state 
					procs_state.GRANTED=true; 
				else// if it is not,
				{
					
					RN[rank]=RN[rank]+1; // update its RN by 1
					printf("Process with rank %d and sequence number %d is requesting critical section\nBroadcast message (%d:%d)\n", rank, RN[rank], rank, RN[rank]);
					for(int i=0; i<size; i++) //send request to the other process with its current RN
						if(i != rank)
						MPI_Send(&RN[rank], 1, MPI_INT, i, REQUESTTAG, MPI_COMM_WORLD);
					
					procs_state.REQUESTED=true; //then go to the requested state 
					
				}
			}
			else  // if the current time is larger than simulation time 
			{
				sim_flag = false; // stop simulation 
				int k;
				for(k=0;k<size;k++)// send STOP command to the other process 
				{
				if(k!=rank)
				MPI_Isend(&FINALIZE,1,MPI_INT,k,STOP,MPI_COMM_WORLD,&request);
				}
				//MPI_Finalize();
				
			}
			
			
			
		}
		if(procs_state.REQUESTED==true) // if the procs state is REQUESTED
		{
			
			flag_waiting2 = true;//assign waiting flag  as true
			waiting_time2 = rand() % (max_wait+1); // generate waiting time between 0 and 5
			wait_finaltime2 = MPI_Wtime() + waiting_time2; //assign waiting_time2
			while(flag_waiting2)
			{
			
				MPI_Iprobe(MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&message_flag,&status); // check if there is any message is available
				usleep(2); // sleep 2 microseconds to check whether or not any message is available
				if(message_flag) //if there is any message is avaiable 
				{
					if( status.MPI_TAG == REQUESTTAG) // if the message is coming with REQUESTTAG
					{
					MPI_Recv(&REQ_INFO, 1, MPI_INT, MPI_ANY_SOURCE, REQUESTTAG, MPI_COMM_WORLD, &status); //receive a message from requested processes 
							
					if(REQ_INFO> RN[status.MPI_SOURCE]) // update RN of the requested process with its SN 
							
						RN[status.MPI_SOURCE] = REQ_INFO; 
							
						printf("Rank %d received critical section request from rank %d\n", rank, status.MPI_SOURCE); // display request of the process 
					}
					else if(status.MPI_TAG == TOKENTAG) // if the message is coming with TOKENTAG
					{
				
					MPI_Recv(&token, 1, ptype, MPI_ANY_SOURCE, TOKENTAG, MPI_COMM_WORLD, &status); 
					printf("Rank %d has received the token from Rank %d and entering into critical section\n", rank, status.MPI_SOURCE);// display token exchange 
					procs_state.GRANTED=true; // go to the GRANTED state 
					owner_flag = 1; // assign owner flag as 1 
					}
				}
			
			
			if( MPI_Wtime() >= wait_finaltime2)// if the current time is larger than max_wait than
				flag_waiting2 = false; 
			}
		
		}
		if(procs_state.GRANTED==true)  // if the current state is GRANTED
		{
			
			cs_endTime = MPI_Wtime() + 2.0; // wait 2 seconds in critical section when the process has token 
			while( MPI_Wtime() < cs_endTime ) 
			{
				
					
					MPI_Iprobe( MPI_ANY_SOURCE, REQUESTTAG, MPI_COMM_WORLD, &message_flag, &status);// check if there is any message 
					usleep(2);//sleep 2 microseconds for checking each message 
					if(message_flag)
					{
						if( status.MPI_TAG == REQUESTTAG)// if the message is coming with REQUESTTAG
						{
						MPI_Recv(&REQ_INFO, 1, MPI_INT, MPI_ANY_SOURCE, REQUESTTAG, MPI_COMM_WORLD, &status); // receive request from the requested processes 
						
						if(REQ_INFO> RN[status.MPI_SOURCE])// if the SN of the requested process is larger than RN of requested process 
						
							RN[status.MPI_SOURCE] = REQ_INFO; //update its RN
						
						printf("Rank %d received critical section request from rank %d\n", rank, status.MPI_SOURCE);//display request 
						}
					}
				
			}
			//After critical section 
			
			token.LN[rank] = RN[rank]; //update LN of the token_owner
			for(int i=0; i<size; i++) // check  if there is any other process waiting the token in the Q
			{
				if( i != rank && RN[i] == token.LN[i]+1 ) 
				{
					inside=0;
					
					for(int j=0; j<counter; j++)
					{
						if(token.Q[j] == i)
						{
							inside = 1;
							j = counter;
						}
						
					}
					if(inside==0) // if the process is not in Q, add that process to Q
					{
						token.Q[counter] = i;
						counter++;
					}
				}
			}
			printf("Rank %d has exited critical section\n", rank); //display exit of the token_owner
			if(counter > 0) // if any process is waiting in the Q
			{
				//send token to the process which is in the top of Q
				token_owner = token.Q[0]; 
				counter=counter-1;				
				for(int i=0; i<counter; i++)
				token.Q[i] = token.Q[i+1];
				printf("Rank %d is sending the token to rank %d\n", rank, token_owner);
				MPI_Send(&token, 1, ptype, token_owner, TOKENTAG, MPI_COMM_WORLD);
				owner_flag = 0;
				
			}
			
			procs_state.RELEASED=true; // Then, go to the RELEASED state
		}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
}











