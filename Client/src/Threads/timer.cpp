#include "globals.hpp"

void timer_thread(){

   std::list<job> temp_workQ;
   while (true) {
        
        { // Sleeping the thread till data in TimerQ
            std::unique_lock<std::mutex> lock_timer(mtx_Timer);

            // Wait until TimerQ has thejobs to check
            cv_timer.wait(lock_timer, [] {
                return !TimerQ.empty();
            });
        }

        auto now = std::chrono::steady_clock::now();
        temp_workQ.clear();

        // Process expired thejobs from the front of TimerQ
        mtx_Timer.lock();
        for (auto it = TimerQ.begin(); it != TimerQ.end(); /* no increment here */) {
            auto &thejob= *it;

            // Check if the the thejob has expired
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - thejob.timestamp) > threshold) {
                // Mark the thejob for resend
                thejob.resend = true;

                //Move the jobs to temporary WorkQ
                temp_workQ.push_back(thejob);

                // Remove from TimerM and erase from TimerQ
                TimerM.erase(thejob.client_id);
                it = TimerQ.erase(it);  // Erase returns the next iterator

                std::cout<<"Removed from TimerQ and added to WorkQ by Timer\n";

            }
            else {
                // Since TimerQ is sorted by insertion time, we can stop here
                break;
            }

            now = std::chrono::steady_clock::now();  // Update current time for the next check
        }
        mtx_Timer.unlock();

        // Move the thejob to WorkQ for reprocessing
        for (auto it = temp_workQ.begin(); it != temp_workQ.end(); /* no increment here */)
        {            
            mtx_WorkQ.lock();
            WorkQ.push_back(*it);
            mtx_WorkQ.unlock();
            
            it=temp_workQ.erase(it);// update statement of loop
        }

        //For Periodic check
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    }//infinite looping
}