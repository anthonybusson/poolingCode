# poolingCode
Code of the simulator used in the article "Performance evaluation of OFDMA and Aggregation downlink stateless service disciplines in Wi-Fi networks"
The tracefile are in the traces folder. Each line is a frame and contains four field: time, packet size, destination ip address, phyiscal transmission rate. 

Once the code has been downloaded, use the make command to compile. 

prompt$ make


To use the simulator the command is:
prompt$ ./pooling sce algo 

The possible values for sce are: 

5: two-stations scenario 

6: mult-destinations scenario

7: traces 

The possible values for algo are: 

0: FIFO (one frame at a time in the FIFO order)

1: FIFO pooling (can be used only with sce 5 and 6) 

5: FIFO Max Pooling (can be used only with sce 5 and 6)

4: MAX Pooling (can be used only with sce 5 and 6)

6: MAX Pooling (can be used only with sce 7)

7: FIFO Max Pooling (can be used only with sce 7)

8: FIFO pooling (can be used only with sce 7) 

