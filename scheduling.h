int updateOfdmaPacketList(int* typeList, int* packetList);
void updateList(int* subset, int len);
void allSubset(int pos, int len, int* subset, int* array, int arraySize);
void allCombination(int* packetList, int len, int* combination, int nbOfPackets, int* bestAllocation, double* ptrBestScore);
double computeOfdmaTransmissionTime(int* packetList, int* allocation, int size);
double computeAggTransmissionTime(int* packetList, int size);
double computeOverheadPacketRatio(int* packetList, int* allocation, int size);
double computeBestAllocation(int* packetList, int* allocation, int size);




void setServicesFIFO(int* packetList);
void setServicesFIFOpooling(int* packetList);
void setServicesOpt(int* packetList);
void setServicesOFDMA(int* packetList);
void setServicesAgg(int* packetList);
void setServicesFIFOMaxPool(int* packetList);
double setServicesRealisticMaxPool(int* packetList);
double setServicesRealisticFifoMaxPool(int* packetList);
double setServicesRealisticFifoPooling(int* packetList);



