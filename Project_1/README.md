1. Components of an event
   - an event will be a tuple or similar data structure containing three elements
       - string containing unique name of the job
       - time of the event in comparison to the beginning of time
       - number that denotes the event type
           - 0 job arrival
           - 1 cpu finished
           - 2 disk1 arrival event
           - 3 disk1 finish event
           - 4 disk2 arrival event
           - 5 disk2 finish event
           - 6 network arrival event
           - 7 network finish event
           - 8 cpu arrival event
           - 9 sim start event
           - 10 sim end event
 
2. Device Int Array
   - I will represent whether a device is being used by flipping numbers in an array
   - 0 empty, 1 for occupied
       - So when a device finished event is processed by the CPU it will find the corresponding device in the array and change it to 0
       - When I/O or CPU is empty it will take in an event
           -change entry in the device array to 1
           -create a finished event with a random int added to the time and send it to the queue
 
3. Job generator
   - int counter to provide a unique number for each job
   - srand used to generate a random number between predefined ARRIVEMIN and ARRIVEMAX
   - creates an event using current time + srand time
 
4. Priority Queue
   - Keeps track of time in the system, as the events are processed the time is updated with the value contained in the tuple
   - Event 0
       - passes event to CPU
   - Event 1,2,3,4,5,6,7,8
       - changes position for corresponding device to 0
       - checks if device queue is not empty
       - pops off first item in queue and creates a device finish and a cpu arrival event
   - Time is calculated as follows, as events are processed a counter increments
       - when the time at top of the queue is greater than the current time we multiply the counter by the difference in time
           - example, if we have 3 jobs in the queue that arrived at time 0, then the next event is at time 20, that means three events were in the queue until time 20 so we add 3 * 20 to the queue time
           - then we divide this sum by total runtime which will give us the average
 
5. CPU, DISK1, DISK2, Network
   - When job arrives at device it will check device array is 0(empty) or busy(1)
       - If device is full
           - add tuple to back of queue
       - If device is empty
           - Change device to 1
           - pass in tuple
           - srand will create a random int, it will be added to cpubusytime and time in tuple will be current + srand
           - create device finished event eg (Job1, 85, 1)
           - create cpu arrival event
 
6. Probability
   - 0-20 is quit probability
   - 21-75 is probability that it will go to a disk
   - 76-100 is network probability
   - I just used conditional probability to calculate the percentages for this program
 
Time is advanced with next event time progression which will be dictated by events on the main/priority queue. Every time an item on the queue is processed that will become the new current time.
 
The priority queue is mainly meant to keep track of time. The events are going to a tuple in the form of (string, int, int), where the string is the unique name of the job, the first int is the time in relation to 0 and the second int indicates what type of event.
 
The sources for the queue will come for the job generator which will trigger after the job arrived event has been processed on the priority queue. Everything else processed on the queue indicates when a device has finished. Events for finished events will happen after the tuple has been passed into the function representing each device.
 
The time is set by the function representing the individual devices, they will use srand to generate a random integer between a predefined min and max and add that to the current time.
 
Events are removed from the priority queue when they have the lowest time value.
 
New processes are created after the job arrival event has been processed by the priority queue.
 
Start time is determined by taking current time (from the priority queue) and adding a random integer using srand.
 
7. Statistics
   - I made the decision when calculating utilization, if the job did not finish then I did not include it in the utilization time. Otherwise I would have to subtract the time of the first job assigned to the component.
   - To compute average queue length I simply added up the size of the queue times the difference in time for the components and then divided by the total time of the system.
   - It was done is a chunk at the end using a mapping and several ints that tracked variables across the system, it could have done more elegantly I could fix with more time
 
Post Project
   - Doing several test runs I noticed that if my peripherals were not significantly slower than the CPU, they would almost never be busy
   - In terms of inter arrival times I would surmise that the most reasonable amount of time is the time it would take to handle interrupts (on a real computer), I'm not sure how you would model that in a discrete event simulation.
       - If it's a simulation I guess we could try to find the best balance between average response time of the cpu and get it as close as possible to the average response time of the peripherals? It doesn't make sense that it's uniform because the CPU is always going to have a higher queue in comparison.
   -Issues
       - Keeping track of the statistics was quite difficult without it becoming verbose, if I placed all those numbers into an array maybe it would make sense but then it reduces readability.

