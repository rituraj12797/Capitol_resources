#include<iostream>
#include<vector>
#include<chrono>
#include<atomic>
#include<thread>
// #include "internal_lib.h"    // do not import the library it self ==> include the header files this library uses 
#include "lf_queue.h"
#include "imp_macros.h"
#include "thread_utils.h"

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



int main() {
	std::cout<<" this is running \n";

	// a writer thread 

	internal_lib::LFQueue<Order> channel(5); // our queue through whihc these 2 threads will communicate ==> ring buffer of size 5

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
    
    auto t1 = internal_lib::createAndStartThread(2, "writer thread", writerFunction, std::ref(channel));
    auto t2 = internal_lib::createAndStartThread(1, "reader thread", readerFunction, std::ref(channel));

	if(t1 != nullptr) {
		t1->join();
	}

	if(t2 != nullptr) {
		t2->join();
	}

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