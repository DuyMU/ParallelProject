#pragma offload_attribute (push, _Cilk_shared)
#include<iostream>
#include<string.h>
#include<fstream>
#include<stdlib.h>
#include<list>
#include<algorithm>
#include<omp.h>
#include<vector>
//#include<mutex>
#include<ctime>
#include "offload.h"
#pragma offload_attribute (pop)
using namespace std;

struct trans{
	list<int> itemSet;
	int frequency;
};

//mutex mtx; //mutex for critical section
pthread_mutex_t lock;
int maxThreads;

double t_comp = 0.0;
double t_off = 0.0;

//declare function
void initData(list<trans> &data, const char*inputFileName, int &maxItem);
void printData(list<trans> data);
void C1(list<trans> &C, int maxItem);
void L1(list<trans>data, list<trans>&C, list<trans>&L, int minSupp);
void pruneData(list<trans>&data, list<trans>L);
bool isExistInL1(int n, list<trans>L);
void generateC(list<trans> &C, list<trans> L);
bool checkCompatibility(trans A, trans B);
//function for prune
void prune(list<trans> &C, list<trans> L);
bool compareTwoList(list<int> A, list<int> B);
bool isTransValid(trans A, list<trans> L);
bool isExistInL(list<int> A, list<trans> L);
bool compareTwoList(list<int> A, list<int> B);
///=================
void scanData(list<trans> data, list<trans> &C, list<trans> &L, int minSupp);
bool isSubset(list<int> data, list<int> subset);


const string currentDateTime();
// main function==============================
int main(int argc, char *argv[]){
	//cout<<"start time: "<<currentDateTime()<<endl;
	string startTime = currentDateTime();
	



	double userMinSupp;	
	char *inputFileName;
	int minSupp;
	int maxItem=0;
	list<trans> data;
	list<trans> L;
	list<trans> C;
	
	if(argc<3){
		cout<<"Find association rule:\n";
		cout<<"Usage: ./apriori [data set] [min support]\n";
	return 0;
	}
	userMinSupp=atof(argv[2]);
	inputFileName=argv[1];
	maxThreads=atof(argv[3]);
	//==============================
	//code to init data and count maxItem
	initData(data, inputFileName, maxItem);
	//printData(data);
	//count minSupp	
	cout << "------------" << userMinSupp << "-----------" << endl;
	if((userMinSupp*data.size()/100)>(double(int(userMinSupp*data.size()/100)))){
		minSupp=int(userMinSupp*data.size()/100)+1;
	}else{
		minSupp=int(userMinSupp*data.size()/100);
	}
	//===============================
	cout << "------------" << minSupp << "-----------" << endl;

	bool firstLoop=true;
	int index=2;
	while(true){
		if(firstLoop==true){
			C1(C, maxItem);
			L1(data, C, L, minSupp);
			pruneData(data, L);
			cout<<"L1\n";
			//printData(L);
			firstLoop=false;
		}else{
			cout<<"Generating C"<<index<<endl;
			generateC(C, L);
			if(C.empty()) break;
			//printData(C);

			cout<<"pruning"<<endl;
			
			prune(C, L);
			//printData(L);

			if(C.empty()) break;
			cout<<"Scanning Data"<<endl;
			scanData(data, C, L, minSupp);
			if(L.empty()) break;
			cout<<"\nL"<<index<<"\n";
			//printData(L);
			index++;
		}
	}

string stopTime = currentDateTime();

cout<<"start time:	"<<startTime<<endl;
cout<<"stop time:	"<<stopTime<<endl;
cout<<"Max threads: "<<maxThreads<<endl;
cout<<"data size: "<<data.size();
}//end main

const string currentDateTime(){
	time_t	now = time(0);
	struct	tm tstruct;
	char	buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}




//=============================================

void initData(list<trans> &data, const char *inputFileName, int &maxItem){
	ifstream fp;
	fp.open(inputFileName);
	string line;
	char *tempLine;
	char *token;
	if(fp.is_open()){
		while(getline(fp, line)){
			trans temp;
			temp.frequency=0;
			//convert string to char*
			tempLine= new char[line.length()+1];
			strcpy(tempLine, line.c_str());
			token=strtok(tempLine, " ");
			//token=strtok(NULL, ",");
			while(token!=NULL){
				if(atoi(token)>maxItem){
					maxItem=atoi(token);
				}
				
				temp.itemSet.push_back(atoi(token));
				
				token=strtok(NULL, " ");
			}
			data.push_back(temp);
			delete[] tempLine;		
		}
		
	}else{
		printf("Can't open input file");
	}
}//end initData

void printData(list<trans> data){
	for(list<trans>::iterator transIter=data.begin(); transIter!=data.end(); transIter++){
		for(list<int>::iterator itemSetIter=transIter->itemSet.begin(); itemSetIter!=transIter->itemSet.end(); itemSetIter++){
			cout<<*itemSetIter<<" ";
		}
		cout<<"====>frequency:"<<transIter->frequency;
		cout<<endl;
	}
}//end printData

void C1(list<trans> &C, int maxItem){
	int i;
	for(i=0; i<maxItem; i++){
		trans temp;
		temp.frequency=0;
		
		temp.itemSet.push_back(i+1);
		C.push_back(temp);	
	}
}//end C1


void L1(list<trans>data, list<trans>&C, list<trans>&L, int minSupp){
	list<trans>::iterator dataIter;
	list<trans>::iterator CIter;
	trans temp;
	for(dataIter=data.begin(); dataIter!=data.end(); dataIter++){
		for(CIter=C.begin(); CIter!=C.end(); CIter++){
			if(find(dataIter->itemSet.begin(), dataIter->itemSet.end(), *(CIter->itemSet.begin()))!=dataIter->itemSet.end()){
				CIter->frequency++;
			}
		}
	}
	while(!C.empty()){
		temp=*(C.begin());
		C.pop_front();
		if(temp.frequency>=minSupp){
			L.push_back(temp);
		}
	}
}//end L1



void pruneData(list<trans>&data, list<trans>L){

	vector<list<trans>::iterator>elements;
	for(list<trans>::iterator dataIter = data.begin(); dataIter != data.end(); dataIter++){
		elements.push_back(dataIter);	
	}

	int numThreads = omp_get_max_threads();
	omp_set_num_threads(maxThreads);
	#pragma offload target (mic : 0 )
	{
		#pragma omp parallel
		{
		#pragma omp for nowait
		for(int i = 0; i<elements.size(); i++){	
		
			list<int>::iterator itemSetIter=elements[i]->itemSet.begin();
			while(itemSetIter!=elements[i]->itemSet.end()){
				if(isExistInL1(*itemSetIter, L)){
					itemSetIter++;	
				}else{
					itemSetIter=elements[i]->itemSet.erase(itemSetIter);
				}
			}
			
			if(elements[i]->itemSet.size() < 2){
				//mtx.lock();
				pthread_mutex_lock(&lock);
				data.erase(elements[i]);
				pthread_mutex_unlock(&lock);
				//mtx.unlock();
			}
		}
	}
//
//	dataIter=data.begin();
//	while(dataIter!=data.end()){
//		if(dataIter->itemSet.size()<2){
//			dataIter=data.erase(dataIter);
//		}else{
//			dataIter++;
//		}
//	}

}//end pruneData

bool isExistInL1(int n, list<trans>L){
	list<trans>::iterator LIter=L.begin();
	while(LIter!=L.end()){
		if(*(LIter->itemSet.begin())==n){
			return true;
		}
		LIter++;
	}
	return false;
}

void generateC(list<trans> &C, list<trans> L){
	C.clear();
	
	vector<list<trans>::iterator>elements;
	for(list<trans>::iterator LIter = L.begin(); LIter != L.end(); LIter++){
		elements.push_back(LIter);	
	}

	int numThreads = omp_get_max_threads();
	omp_set_num_threads(maxThreads);
	#pragma offload target (mic : 0)
	{
		#pragma omp parallel
		{
		#pragma omp for nowait
		for(int i = 0; i<elements.size(); i++){
			list<trans>::iterator tempL1 = elements[i];
			list<trans>::iterator tempL2 = tempL1;
			tempL2++;
			for(int j = i; j<L.size()-1; j++){
				if(checkCompatibility(*tempL1, *tempL2)){
					if(*(--tempL1->itemSet.end())>*(--tempL2->itemSet.end())){
						trans temp=*tempL2;
						temp.frequency=0;
						temp.itemSet.push_back(*(--tempL1->itemSet.end()));
						//input this transation into C
						//mtx.lock();
						pthread_mutex_lock(&lock);
						C.push_back(temp);
						pthread_mutex_unlock(&lock);
						//mtx.unlock();
					}else{
						trans temp=*tempL1;
						temp.frequency=0;
						temp.itemSet.push_back(*(--tempL2->itemSet.end()));
						//input this transaction into C
						//mtx.lock();
						pthread_mutex_lock(&lock);
						C.push_back(temp);
						pthread_mutex_unlock(&lock);
						//mtx.unlock();
					}	
				}
				tempL2++;
			}
			//tempL1++;
		}//end for
		}//end #pragma omp parallel
	}
	
}//end gennerateC


bool checkCompatibility(trans A, trans B){
	bool compatible=true;
	list<int>::iterator tempA=A.itemSet.begin();
	list<int>::iterator tempB=B.itemSet.begin();
	int i;
	for(i=0; i<A.itemSet.size()-1; i++){
		if(*tempA!=*tempB){
			compatible=false;
			break;
		}
		tempA++;
		tempB++;
		//cout<<"return false";
	}
	return compatible;
}//end checkCompatibility

void prune(list<trans> &C, list<trans> L){
	
	vector<list<trans>::iterator>elements;
	for(list<trans>::iterator CIter = C.begin(); CIter != C.end(); CIter++){
		elements.push_back(CIter);	
	}

	int numThreads = omp_get_max_threads();
	omp_set_num_threads(maxThreads);
	#pragma offload target (mic : 0)
	{
		#pragma omp parallel
		{
		#pragma omp for nowait
		for(int i = 0; i<elements.size(); i++){
			if(isTransValid(*elements[i], L)==true){
				//CIter++;
				//do nothing
			}else{
				//mtx.lock();
				pthread_mutex_lock(&lock);
				elements[i]=C.erase(elements[i]);
				pthread_mutex_unlock(&lock);
				//mtx.unlock();
			}
		}
		}//end #pragma omp parallel
	}
}



bool isTransValid(trans A, list<trans> L){
	int i;
	for(i=0; i<A.itemSet.size(); i++){
		list<int> temp=A.itemSet;
		list<int>::iterator tempIter=temp.begin();
		advance(tempIter, i);
		temp.erase(tempIter);
		if(isExistInL(temp, L)==false){
			return false;
		}
	}
	return true;
}
bool isExistInL(list<int> A, list<trans> L){
	for(list<trans>::iterator LIter=L.begin(); LIter!=L.end(); LIter++){
		if(compareTwoList(A, (LIter->itemSet))){
			return true;
		}
	}
	return false;
}
bool compareTwoList(list<int> A, list<int> B){
	if(A.size()!=B.size())return false;
	list<int>::iterator tempA=A.begin();
	list<int>::iterator tempB=B.begin();
	for(tempA=A.begin(); tempA!=A.end(); tempA++){
		if(*tempA!=*tempB){
			return false;
		}
		tempB++;
	}
	return true;
}

void scanData(list<trans>data, list<trans> &C, list<trans> &L, int minSupp){
	// Them
	// 1
	_Cilk_shared vector<trans *, __offload::shared_allocator<trans *> > * _Cilk_shared v;
	v = new (_Offload_shared_malloc(sizeof(vector<trans *>)))
          _Cilk_shared
          vector<trans*, __offload::shared_allocator<trans*> >(C.size()); 
	
	// 2
	for(list<trans>::iterator CIter = C.begin(), int i = 0; CIter != C.end(); CIter++, i++){
		(*v)[i] = &(*CIter);
	}
	
	
	
	//--------------------------------------------------------------------------------------------
	vector<trans *>elements;
	for(list<trans>::iterator CIter = C.begin(); CIter != C.end(); CIter++){
		elements.push_back(&(*CIter));
	}
	
	int numThreads = omp_get_max_threads();
	omp_set_num_threads(maxThreads);
	//#pragma offload target(mic : 0)
	#pragma omp parallel
	{
	#pragma omp for nowait
	for(int i=0; i<elements.size(); i++){
		for(list<trans>::iterator dataIter=data.begin();
				dataIter!=data.end(); dataIter++){
			if(isSubset(dataIter->itemSet, elements[i]->itemSet)==true){
				elements[i].frequency++;
			}
		}
	}//end #pragma omp for nowait
	}//end #pragma omp parallel
	}
	L.clear();
	L=C;
	C.clear();
	list<trans>::iterator LIter=L.begin();
	while(LIter!=L.end()){
		if(LIter->frequency>=minSupp){
			LIter++;
		}else{
			LIter=L.erase(LIter);
		}
	}
}

bool isSubset(list<int> data, list<int> subset){
	if(data.size()<subset.size()){
		return false;
	}

	for(list<int>::iterator subsetIter=subset.begin(); subsetIter!=subset.end(); subsetIter++){
		if(find(data.begin(), data.end(), *subsetIter)==data.end()){
			return false;
		}
	}
	return true;
}







