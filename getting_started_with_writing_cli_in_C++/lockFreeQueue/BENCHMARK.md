Main Thread Executing 
Thread [ std writer ] pinned to Core 3
Thread [ std reader ] pinned to Core 4
 ========================================= BENCHMARK PERFORMANCE FOR  std::queue + mutex ================================ 


 WRITE LATENCIES 
 P50 : 83
 P90 : 552
 P99 : 1648
 P99.9 : 8467
 P99.99 : 18222
 P99.999 : 107752
 READ LATENCIES 
 P50 : 53
 P90 : 446
 P99 : 1750
 P99.9 : 6841

 P99.99 : 24033
 P99.999 : 209551

========================================================================================================


Thread [ writer to lq_queue ] pinned to Core 6
Thread [ writer to lq_queue ] pinned to Core 5
 ========================================= BENCHMARK PERFORMANCE FOR  LF_queue ================================ 


 WRITE LATENCIES 
 P50 : 18
 P90 : 22
 P99 : 25
 P99.9 : 42
 P99.99 : 1023
 P99.999 : 39145
 READ LATENCIES 
 P50 : 17
 P90 : 18
 P99 : 20
 P99.9 : 44

 P99.99 : 209
 P99.999 : 2473

========================================================================================================
