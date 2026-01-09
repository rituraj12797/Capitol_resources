#include<iostream>
#include<vector>
#include<chrono>
#include<atomic>
#include<thread>
// #include "internal_lib.h"    // do not import the library it self ==> include the header files this library uses 
#include "lf_queue.h"
#include "imp_macros.h"
#include "thread_utils.h"

#include<new>
#include<queue>
#include<mutex>
#include<algorithm>


struct Order {
	int stock_id;
	int quantity;
	bool buy;

	Order(int id = 5, int number_of_stocks = 6, bool is_buy = false) {
		stock_id = id;
		quantity = number_of_stocks;
		buy = is_buy;
	}
};

// void writerFunction(internal_lib::LFQueue<Order> &comm) noexcept {

// 	// let us do say 20 fast insertions and 20 slow reads 

// 	int cnt = 20;
// 	auto write_ptr = comm.getNextWriteIndex();

// 	while(true) {
// 		while(write_ptr != nullptr) {
// 			cnt--; // dicrenment count
// 			*write_ptr = Order(20 - cnt); // id ==> 0 1 2 3 4  . . . . . 20
// 			std::cout<<" Written 20 - cnt \n";
// 			comm.updateWriteIndex(); // tries to update the write index 
// 			write_ptr = comm.getNextWriteIndex() ;
// 		}
// 		write_ptr = comm.getNextWriteIndex();
// 	}

// }

void writerFunction(internal_lib::LFQueue<Order> &comm) noexcept {
    int cnt = 0;
    // simulate for 5 orders 
    while(cnt < 20) {
        
        auto* write_ptr = comm.getNextWriteIndex();

        // BUSY WAIT  ==> Backpressure preventing it from writing 
        // if the queue is full (nullptr) we stay in this loop until space opens up.
        while(write_ptr == nullptr) {
            // "Market" is too fast, reader is too slow.

            // what we do ?? ==> Sleep and try again later ? or  Kepe trying again and again and again ? 

            // ---------------------------------------------------------
        	// STRATEGY: OPTIMAL WAITING (YIELD VS SLEEP VS SPIN)    == USED GEN AI HERE TO WRITE WHY WE USED yield 
       		// ---------------------------------------------------------
        
        	// 1. WHY NOT BUSY SPIN? (while(true) {})
        	//    Pure spinning burns 100% CPU on the core. This generates heat and 
        	//    consumes power. Crucially, if we are on a hyper-threaded core, 
        	//    spinning starves the *other* thread (e.g., the Reader) that we 
        	//    are desperately waiting for.
        
        	// 2. WHY NOT SLEEP? (std::this_thread::sleep_for)
        	//    Sleep puts the thread into a 'WAITING' or 'SUSPENDED' state.
        	//    The OS removes it from the run queue. Waking it up requires:
        	//      a. An interrupt or timer event.
        	//      b. The OS Scheduler to decide to reschedule us.
        	//      c. Context switching back to 'READY' then 'RUNNING'.
        	//    In HFT, this "Wake Up Latency" (often >10-50 microseconds) is an eternity.

        	// 3. THE SOLUTION: YIELD (std::this_thread::yield)
        	//    Yield tells the OS: "I am done with my current time slice, but I am still
        	//    ready to work."
        	//    - State remains: READY (not SUSPENDED).
        	//    - Action: Moves this thread to the back of the generic CPU run queue.
        	//    - Result: Allows the other thread to run immediately, but we can return 
        	//      to execution much faster (nanoseconds/microseconds) than waking from sleep.


            std::this_thread::yield(); 
            
            // after running again eed to check if we can send now or not 
            write_ptr = comm.getNextWriteIndex();
        }

        //  We broke out of the loop that means write_ptr is valid now and we can push an entry in the queue 
        *write_ptr = Order(cnt, 100, true); 
        std::cout << "[writer] Order Sent: " << cnt << "\n";

        comm.updateWriteIndex();
        
        cnt++;
        
    }
    std::cout << "[writer] Closed. All orders sent.\n";
}


void readerFunction(internal_lib::LFQueue<Order> &comm) noexcept {
    int cnt = 0;
    while(cnt < 20) {
        auto* read_ptr = comm.getNextReadIndex();

        // spin until data arrives
        while(read_ptr == nullptr) {
            std::this_thread::yield();
            read_ptr = comm.getNextReadIndex(); // refresh status
        }

        // data found ===> process it
        std::cout << "\t[reader] Processed: " << read_ptr->stock_id << "\n";

        comm.updateNextReadIndex();
        cnt++;
    }
    std::cout << "[reader] Stopped.\n";
}

// void readerFunction(internal_lib::LFQueue<Order> &comm) noexcept {


// 	auto read_ptr = comm.getNextReadIndex();

// 	while(true) {
// 		while(read_ptr != nullptr) {
// 			std::cout<<" Read  "<<read_ptr->stock_id<<"\n";
// 			comm.updateNextReadIndex(); // tries to update the write index 
// 			read_ptr = comm.getNextReadIndex() ;
// 		}
// 	}


// }	

void showBenchmark(std::string text, std::vector<std::chrono::duration<double,std::nano>>& write, std::vector<std::chrono::duration<double,std::nano>>& read ) {
    std::sort(write.begin(),write.end());
    std::sort(read.begin(),read.end());

    int ind_fifty = write.size()/2;
    int ind_nine_zero = (9*write.size())/(10);
    int ind_ninty_nine = (99*write.size())/(100);
    int ind_nine_nine_nine = (999*write.size())/(1000);
    int ind_nine_nine_nine_nine = (write.size()/10000)*9999;
    int ind_nine_nine_nine_nine_nine = (write.size()/100000)*99999;
    int ind_nine_nine_nine_nine_nine_nine = (write.size()/1000000)*999999;

    std::cout<<" ========================================= BENCHMARK PERFORMANCE FOR "<<text<<"================================ \n\n\n";
    /* ======================= BENCHMARK RESULTS NOW ===============================  */
    std::cout<<" WRITE LATENCIES \n";
    std::cout<<" P50 : "<<write[ind_fifty].count()<<"\n";
    std::cout<<" P90 : "<<write[ind_nine_zero].count()<<"\n";
    std::cout<<" P99 : "<<write[ind_ninty_nine].count()<<"\n";
    std::cout<<" P99.9 : "<<write[ind_nine_nine_nine].count()<<"\n";
    std::cout<<" P99.99 : "<<write[ind_nine_nine_nine_nine].count()<<"\n";
    std::cout<<" P99.999 : "<<write[ind_nine_nine_nine_nine_nine].count()<<"\n";



    std::cout<<" READ LATENCIES \n";
    std::cout<<" P50 : "<<read[ind_fifty].count()<<"\n";
    std::cout<<" P90 : "<<read[ind_nine_zero].count()<<"\n";
    std::cout<<" P99 : "<<read[ind_ninty_nine].count()<<"\n";
    std::cout<<" P99.9 : "<<read[ind_nine_nine_nine].count()<<"\n\n";
    std::cout<<" P99.99 : "<<read[ind_nine_nine_nine_nine].count()<<"\n";
    std::cout<<" P99.999 : "<<read[ind_nine_nine_nine_nine_nine].count()<<"\n";

    std::cout<<"\n========================================================================================================\n\n\n";
}



int main() {
	std::cout<<" Main Thread Executing \n";

	// a writer thread 

	// internal_lib::LFQueue<Order> channel(5); // our queue through whihc these 2 threads will communicate ==> ring buffer of size 5

	// ---------------------------------------------------------
    // WHY std::ref(channel)?     GENERATIVE AI USED HERE TO PROPERLY DESCRIBE THE SCENARIO WITH EASE.
    // ---------------------------------------------------------
    
    // 1. WHY NOT PASS BY VALUE? (channel)
    //    If we pass 'channel' directly, the std::thread constructor attempts to COPY it.
    //    Two problems arise:
    //      a. We DELETED the copy constructor in LFQueue to prevent accidental logic bugs.
    //      b. Even if enabled, a copy would create a DUPLICATE queue. The Writer would 
    //         write to Queue A, and the Reader would read from Queue B. They wouldn't talk.

    // 2. WHY NOT PASS ADDRESS? (&channel)
    //    Our functions (writerFunction) accept a Reference (LFQueue&), not a Pointer (LFQueue*).
    //    Passing &channel sends a pointer, causing a compilation type mismatch.

    // 3. HOW std::ref WORKS (The "Copy" Trick)
    //    std::ref creates a lightweight "wrapper" object (std::reference_wrapper).
    //    This wrapper acts like a pointer but behaves like a reference.
    //    - The thread copies the WRAPPER (which is allowed and cheap).
    //    - The wrapper holds the address of the original 'channel'.
    //    - Result: The thread gets access to the ORIGINAL memory without triggering a deep copy.

    // 4. WHY IS THIS SAFE? (No Dangling Pointer)
    //    We are passing a reference to a variable ('channel') living on the Main Thread's stack.
    //    Usually, this is dangerous if the Main Thread finishes before the child threads.
    //    HOWEVER, we call t1->join() and t2->join() below.
    //    This forces Main to wait until threads are dead before it exits and destroys 'channel'.
    
    // auto t1 = internal_lib::createAndStartThread(2, "writer thread", writerFunction, std::ref(channel));
    // auto t2 = internal_lib::createAndStartThread(1, "reader thread", readerFunction, std::ref(channel));

	// if(t1 != nullptr) {
	// 	t1->join();
	// }

	// if(t2 != nullptr) {
	// 	t2->join();
	// }
    // test set across which test will be performed 

    int set_size = 100000;
    std::vector<Order> test_set(set_size, Order(5,6,false));



    // ======================================= STD QUEUE + MUTEX LOCKS ==============================================
    std::queue<Order> std_queue;
    std::mutex mtx; 
    std::atomic<bool> done = {false};

    std::vector<std::chrono::duration<double,std::nano>> write_times;
    std::vector<std::chrono::duration<double,std::nano>> read_times;


    // writer lambda 
    auto writer_f = [](std::queue<Order>& std_queue,std::vector<Order>& test_set, std::vector<std::chrono::duration<double,std::nano>>& write_times , std::mutex& mtx, std::atomic<bool>& done ) {
        // write million entries with mutex locks

        for(int i = 0 ; i < 100000; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            mtx.lock();
            std_queue.push(test_set[i]);
            mtx.unlock();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double,std::nano> interval = end - start;
            write_times.push_back(interval);
        }

        done = true;
    };

    // reader lambda
    auto reader_f = [](std::queue<Order>& std_queue, std::mutex& mtx, std::atomic<bool>& done, std::vector<std::chrono::duration<double,std::nano>>& read_times ) {
    int rc = 0;
    while (true) { // Loop forever until we explicitly break
        
        auto start = std::chrono::high_resolution_clock::now();
        mtx.lock();
        
        if (std_queue.empty()) {
            mtx.unlock();
            // If queue is empty AND writer is finished, we can stop.
            if (done) break; 
            
            // Otherwise, writer is still working, so we yield and wait for data
            std::this_thread::yield();
            continue;
        }

        // If we are here, queue has data
        Order entry = std_queue.front();
        std_queue.pop();
        rc += (entry.stock_id > 0 ? 1 : 0);
        mtx.unlock();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double,std::nano> interval = end - start;
        read_times.push_back(interval);
    }
    };


    // writer thread
    auto writer_thread = internal_lib::createAndStartThread(3," std writer ", writer_f, std::ref(std_queue), std::ref(test_set), std::ref(write_times), std::ref(mtx), std::ref(done));
    // reader thread
    auto reader_thread = internal_lib::createAndStartThread(4," std reader ", reader_f, std::ref(std_queue), std::ref(mtx), std::ref(done), std::ref(read_times));
    

    if(writer_thread != nullptr) {
        writer_thread->join();
    }

    if(reader_thread != nullptr) {
        reader_thread->join();
    }


    showBenchmark(" std::queue + mutex ",write_times, read_times);


    // ========================================================   LOCK FREE QUEUE ========================================

    internal_lib::LFQueue<Order> ring_buffer((set_size)); //  we will make the queue of size (2^x) ==> why ==> so that we can optimise the wrap around from usinfg Division ( ~ 40 CPU cycles) to using bitwise operator -> ( 1 CPU cycle )
    // ==>  2^26 ~=67M
    std::vector<std::chrono::duration<double,std::nano>> lfq_write_times;
    std::vector<std::chrono::duration<double,std::nano>> lfq_read_times;

    // write lambda
    auto wf = [](internal_lib::LFQueue<Order>& comm, std::vector<std::chrono::duration<double,std::nano>>& lfq_write_times) {

        int cnt = 0; // I Million Orders
        while(cnt < 100000) {
            auto start = std::chrono::high_resolution_clock::now();
            auto write_ptr = comm.getNextWriteIndex();
            while(write_ptr == nullptr) {
                std::this_thread::yield();
                write_ptr = comm.getNextWriteIndex();
            }   

            *write_ptr = Order(cnt, 100, true); 
            comm.updateWriteIndex();
            cnt++;
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double,std::nano>  interval = end - start;
            lfq_write_times.push_back(interval);
        
        }
    };

    // read lambda
    auto rf = [](internal_lib::LFQueue<Order>& comm, std::vector<std::chrono::duration<double,std::nano>>& lfq_read_times) {
        int cnt = 0;
        while(cnt < 100000) {
            auto start = std::chrono::high_resolution_clock::now();
            auto* read_ptr = comm.getNextReadIndex();
            while(read_ptr == nullptr) {
                std::this_thread::yield();
                read_ptr = comm.getNextReadIndex(); 
            }
            comm.updateNextReadIndex();
            cnt++;
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double,std::nano>  interval = end - start;
            lfq_read_times.push_back(interval);
        }
    };


    // writer thread to lf_queue
    auto wt = internal_lib::createAndStartThread(6," writer to lq_queue ",wf,std::ref(ring_buffer),std::ref(lfq_write_times));
    // reader thread
    auto rt = internal_lib::createAndStartThread(5," writer to lq_queue ",rf,std::ref(ring_buffer),std::ref(lfq_read_times));


    if(wt != nullptr) {
        wt->join();
    }

    if(rt != nullptr) {
        rt->join();
    }

   showBenchmark(" LF_queue ",lfq_write_times, lfq_read_times);



	return 0;


}


/*

First output was like : 

rituraj12797@bellw3thers-pc:~/Capitol_resources/getting_started_with_writing_cli_in_C++/lockFreeQueue/build$ ./lockfreequeue 
 this is running 
Thread [writer thread] pinned to Core 2
[writer] Order Sent: 0
[writer] Order Sent: 1
[writer] Order Sent: 2
[writer] Order Sent: 3
Thread [reader thread] pinned to Core 1
	[reader] Processed: 0
	[reader] Processed: 1
	[reader] Processed: 2
	[reader] Processed: 3
[writer] Order Sent: 4
[writer] Order Sent: 5
[writer] Order Sent: 6
[writer] Order Sent: 7
	[reader] Processed: 4
	[reader] Processed: 5
	[reader] Processed: 6
	[reader] Processed: 7
[writer] Order Sent: 8
[writer] Order Sent: 9
[writer] Order Sent: 10
	[reader] Processed: [writer] Order Sent: 11
8
	[reader] Processed: 9
	[reader] Processed: 10
	[reader] Processed: 11
[writer] Order Sent: 12
[writer] Order Sent: 13
[writer] Order Sent: 14
	[reader] Processed: [writer] Order Sent: 12
	[reader] Processed: 13
15	[reader] Processed: 14

	[reader] Processed: [writer] Order Sent: 15
16
[writer] Order Sent: 	[reader] Processed: 1716
[writer] Order Sent: 18
[writer] Order Sent: 19
[writer] Closed. All orders sent.

	[reader] Processed: 17
	[reader] Processed: 18
	[reader] Processed: 19
[reader] Stopped.

 the overlaping in the printing statements shows that, true parallelism is achieved corectly as the print statements are not atomic so even before one thread completes his writes one interrupts and writes ""

 that 1716 is not actually 1716 but 17 and 16 written together.
*/





































// #include<iostream>
// #include<vector>
// #include<chrono>
// #include<atomic>
// #include<thread>
// #include<new>
// #include<queue>
// #include<mutex>
// #include<algorithm>

// // INCLUDE YOUR NEW OPTIMIZED HEADER HERE
// #include "fast_lf_queue.h" 
// #include "imp_macros.h"
// #include "thread_utils.h"


// struct Order {
//     int stock_id;
//     int quantity;
//     bool buy;

//     Order(int id = 5, int number_of_stocks = 6, bool is_buy = false) {
//         stock_id = id;
//         quantity = number_of_stocks;
//         buy = is_buy;
//     }
// };

// int main() {
//     std::cout<<" Main Thread Executing \n";

//     // Test set size
//     const int NUM_OPS = 1000000;
//     std::vector<Order> test_set(NUM_OPS, Order(5,6,false));


//     // ======================================= 1. STD QUEUE + MUTEX LOCKS ==============================================
//     std::cout << "\n--- Benchmarking std::queue + std::mutex ---\n";
//     std::queue<Order> std_queue;
//     std::mutex mtx; 
//     std::atomic<bool> done = {false};

//     std::vector<std::chrono::duration<double,std::nano>> write_times;
//     std::vector<std::chrono::duration<double,std::nano>> read_times;
//     write_times.reserve(NUM_OPS);
//     read_times.reserve(NUM_OPS);

//     // Writer Lambda (Mutex)
//     auto writer_f = [](std::queue<Order>& std_queue, std::vector<Order>& test_set, std::vector<std::chrono::duration<double,std::nano>>& write_times , std::mutex& mtx, std::atomic<bool>& done ) {
//         for(int i = 0 ; i < NUM_OPS; i++) {
//             auto start = std::chrono::high_resolution_clock::now();
            
//             mtx.lock();
//             std_queue.push(test_set[i]);
//             mtx.unlock();
            
//             auto end = std::chrono::high_resolution_clock::now();
//             write_times.push_back(end - start);
//         }
//         done = true;
//     };

//     // Reader Lambda (Mutex)
//     auto reader_f = [](std::queue<Order>& std_queue, std::mutex& mtx, std::atomic<bool>& done, std::vector<std::chrono::duration<double,std::nano>>& read_times ) {
//         int cnt = 0;
//         while (cnt < NUM_OPS) { 
//             auto start = std::chrono::high_resolution_clock::now();
//             mtx.lock();
            
//             if (std_queue.empty()) {
//                 mtx.unlock();
//                 std::this_thread::yield(); // Backoff
//                 continue;
//             }

//             Order entry = std_queue.front();
//             std_queue.pop();
//             mtx.unlock();
            
//             auto end = std::chrono::high_resolution_clock::now();
//             read_times.push_back(end - start);
//             cnt++;
//         }
//     };

//     auto writer_thread = internal_lib::createAndStartThread(3,"std writer", writer_f, std::ref(std_queue), std::ref(test_set), std::ref(write_times), std::ref(mtx), std::ref(done));
//     auto reader_thread = internal_lib::createAndStartThread(4,"std reader", reader_f, std::ref(std_queue), std::ref(mtx), std::ref(done), std::ref(read_times));
    
//     if(writer_thread != nullptr) writer_thread->join();
//     if(reader_thread != nullptr) reader_thread->join();

//     // Stats for Mutex
//     std::sort(write_times.begin(),write_times.end());
//     std::cout<<" MUTEX P50   : "<<write_times[write_times.size()*0.50].count()<<"\n";
//     std::cout<<" MUTEX P99   : "<<write_times[write_times.size()*0.99].count()<<"\n";
//     std::cout<<" MUTEX P99.9 : "<<write_times[write_times.size()*0.999].count()<<"\n";
//     std::cout<<" MUTEX P99.99 : "<<write_times[write_times.size()*0.9999].count()<<"\n";
//     std::cout<<" MUTEX P99.999 : "<<write_times[write_times.size()*0.99999].count()<<"\n";
//     std::cout<<" MUTEX P99.9999 : "<<write_times[write_times.size()*0.999999].count()<<"\n";


//     // ===================================== 2. FAST LOCK FREE QUEUE ========================================
//     std::cout << "\n--- Benchmarking FastLFQueue (Optimized) ---\n";
    
//     // Size will actomatically round up to next Power of 2 inside constructor
//     internal_lib::FastLFQueue<Order> ring_buffer(NUM_OPS); 

//     std::vector<std::chrono::duration<double,std::nano>> lfq_write_times;
//     std::vector<std::chrono::duration<double,std::nano>> lfq_read_times;
//     lfq_write_times.reserve(NUM_OPS);
//     lfq_read_times.reserve(NUM_OPS);

//     // Writer Lambda (Fast Lock Free)
//     auto wf = [](internal_lib::FastLFQueue<Order>& comm, std::vector<std::chrono::duration<double,std::nano>>& times) {
//         int cnt = 0; 
//         while(cnt < NUM_OPS) {
//             auto start = std::chrono::high_resolution_clock::now();
            
//             // Try to push. If false (full), yield and retry.
//             // Note: In a real system, you might loop tightly a few times before yielding.
//             while(!comm.push(Order(cnt, 100, true))) {
//                  std::this_thread::yield(); 
//             }
            
//             auto end = std::chrono::high_resolution_clock::now();
//             times.push_back(end - start);
//             cnt++;
//         }
//     };

//     // Reader Lambda (Fast Lock Free)
//     auto rf = [](internal_lib::FastLFQueue<Order>& comm, std::vector<std::chrono::duration<double,std::nano>>& times) {
//         int cnt = 0;
//         while(cnt < NUM_OPS) {
//             auto start = std::chrono::high_resolution_clock::now();
            
//             // Peek to check for data
//             Order* data = comm.peek();
//             while(data == nullptr) {
//                 std::this_thread::yield();
//                 data = comm.peek();
//             }

//             // Data is safe to access here because writer won't overwrite it until we pop
//             volatile int val = data->stock_id; // "Process" the data
//             (void)val;

//             // Signal we are done with this slot
//             comm.pop();
            
//             auto end = std::chrono::high_resolution_clock::now();
//             times.push_back(end - start);
//             cnt++;
//         }
//     };

//     // Launch Threads (Lock Free) - Using Cores 5 and 6
//     auto wt = internal_lib::createAndStartThread(5,"LF Writer", wf, std::ref(ring_buffer), std::ref(lfq_write_times));
//     auto rt = internal_lib::createAndStartThread(6,"LF Reader", rf, std::ref(ring_buffer), std::ref(lfq_read_times));

//     if(wt != nullptr) wt->join();
//     if(rt != nullptr) rt->join();

//     std::sort(lfq_write_times.begin(),lfq_write_times.end());
    
//     std::cout<<" LFQ P50   : "<<lfq_write_times[lfq_write_times.size()*0.50].count()<<"\n";
//     std::cout<<" LFQ P99   : "<<lfq_write_times[lfq_write_times.size()*0.99].count()<<"\n";
//     std::cout<<" LFQ P99.9 : "<<lfq_write_times[lfq_write_times.size()*0.999].count()<<"\n";
//     std::cout<<" LFQ P99.99 : "<<lfq_write_times[lfq_write_times.size()*0.9999].count()<<"\n";
//     std::cout<<" LFQ P99.999 : "<<lfq_write_times[lfq_write_times.size()*0.99999].count()<<"\n";
//     std::cout<<" LFQ P99.9999 : "<<lfq_write_times[lfq_write_times.size()*0.999999].count()<<"\n";


//     return 0;
// }