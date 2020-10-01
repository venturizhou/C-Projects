#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <tuple>
#include <fstream>
#include <map>

using namespace std;

// tuple<int, string, int> taskcreator(int &time);
int makejob(int arrivemin, int arrivemax);
//read in text file create mapping of integers
map<string, int> configuration(ifstream &filename);
//if cpu is not busy then pops off cpu queue and creates cpu finish event
tuple<int, int, string> event_finish(deque<tuple<int, int, string>> &device, int device_list[], tuple<int, int, string> event, map<string, int> parameters, int device_num, int &device_time, int &response_time, unsigned int &queuetime);
//generates map to store our statistics
void max_queue(deque<tuple<int, int, string>> device, map<string, double> &stats, int device_num);
//writing to textfile
void write_log(ofstream &filename, tuple<int, int, string> event);
//random test
void test_function(void (*f)(deque<tuple<int, int, string>>, map<string, double> &, int));

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        cout << "Config file required" << endl;
        exit(1);
    }
    else if (argc > 2)
    {
        cout << "Project1 [config file...]" << endl;
        exit(1);
    }
    else
    {
        ifstream config(argv[1]);
        ofstream log("log.txt");
        map<string, int> parameters;
        if (!config.is_open())
        {
            cout << "Unable to open file" << endl;
            exit(1);
        }
        else if (!log.is_open())
        {
            cout << "Unable to write to log.txt" << endl;
            exit(1);
        }
        else
        {
            parameters = configuration(config);
        }
        config.close();
        //setup data structures required for sim
        int device[4] = {};
        int networkjobs = 0, cpujobs = 0, disk1jobs = 0, disk2jobs = 0, cputime = 0, disk1time = 0, disk2time = 0, networktime = 0;
        int cpuresponsetime = 0, disk1responsetime = 0, disk2responsetime = 0, networkresponsetime = 0;
        unsigned int cpuqueuetime = 0, disk1queuetime = 0, disk2queuetime = 0, networkqueuetime = 0, pqtime = 0;
        int pqsizecounter = 1;
        priority_queue<tuple<int, int, string>, vector<tuple<int, int, string>>, greater<tuple<int, int, string>>> pq;
        deque<tuple<int, int, string>> cpu, disk1, disk2, network;
        map<string, double> stats;

        //start sim event a
        pq.push(make_tuple(0, 9, "SIM_START"));
        pq.push(make_tuple(parameters["FINISH"], 10, "SIM_FINISH"));
        pq.push(make_tuple(0, 0, "job#0"));
        srand(parameters["SEED"]);
        int jobcounter = 1;
        //main logic to run while program is running
        while (parameters["INIT"] < parameters["FINISH"])
        {
            pqsizecounter++;
            if(pq.size() > stats["priority_max_queue"]){
                stats["priority_max_queue"] = pq.size();
            }
            tuple<int, int, string> top = pq.top();
            pq.pop();
            //adds by using a counter to keep track of number of events at a given time, then uses the next tuples time to calculate the duration those items have been on the queue
            if (get<0>(pq.top()) > get<0>(top) && get<0>(pq.top())<=parameters["FINISH"]){
                pqtime += pqsizecounter * (get<0>(pq.top())-get<0>(top));
                //checking each time is resonable
                // cout << pqtime << endl;
                pqsizecounter = 1;
            }
            parameters["INIT"] = get<0>(top);
            switch (get<1>(top))
            {
            case 0:
            {
                //test log write
                // log << "This is the real job arrival" << endl;
                write_log(log, top);
                //create new job and push it to the queue
                pq.push(make_tuple(parameters["INIT"] + makejob(parameters["ARRIVE_MIN"], parameters["ARRIVE_MAX"]), 0, "job#" + to_string(jobcounter)));
                jobcounter++;
                get<1>(top) = 8;
                pq.push(top);
                break;
            }
            //cpu finish event
            case 1:
            {
                write_log(log, top);
                device[0] = 0;
                if (!cpu.empty())
                {
                    pq.push(event_finish(cpu, device, top, parameters, 0, cputime, cpuresponsetime, cpuqueuetime));
                }
                int choice = rand() % 100;
                if (choice <= parameters["QUIT_PROB"])
                {
                    break;
                }
                // test to make sure my rand is working properly
                // else if (choice > 100){
                //     get<1>(top) = 6;
                // }
                else if (choice >= (100 - parameters["NETWORK_PROB"]))
                {
                    get<1>(top) = 6;
                }
                else
                {
                    if (disk1.size() < disk2.size())
                    {
                        get<1>(top) = 2;
                    }
                    else if (disk2.size() > disk1.size())
                    {
                        get<1>(top) = 4;
                    }
                    else
                    {
                        if (rand() % 100 + 1 > 50)
                        {
                            get<1>(top) = 2;
                        }
                        else
                        {
                            get<1>(top) = 4;
                        }
                    }
                }
                pq.push(top);
                break;
            }
            case 2:
            {
                write_log(log, top);
                disk1.push_back(top);
                disk1jobs++;
                max_queue(disk1, stats, 1);
                if (device[1] == 0)
                {
                    auto temp = event_finish(disk1, device, top, parameters, 1, disk1time, disk1responsetime, disk1queuetime);
                    pq.push(temp);
                    get<1>(temp) = 8;
                    pq.push(temp);
                }
                break;
            }
            case 3:
            {
                write_log(log, top);
                device[1] = 0;
                if (!disk1.empty())
                {
                    auto temp = event_finish(disk1, device, top, parameters, 1, disk1time, disk1responsetime, disk1queuetime);
                    pq.push(temp);
                    get<1>(temp) = 8;
                    pq.push(temp);
                }
                break;
            }
            case 4:
            {
                write_log(log, top);
                disk2.push_back(top);
                disk2jobs++;
                max_queue(disk2, stats, 2);
                if (device[2] == 0)
                {
                    auto temp = event_finish(disk2, device, top, parameters, 2, disk2time, disk2responsetime, disk2queuetime);
                    pq.push(temp);
                    get<1>(temp) = 8;
                    pq.push(temp);
                }
                break;
            }
            case 5:
            {
                write_log(log, top);
                device[2] = 0;
                if (!disk2.empty())
                {
                    auto temp = event_finish(disk2, device, top, parameters, 2, disk2time, disk2responsetime, disk2queuetime);
                    pq.push(temp);
                    get<1>(temp) = 8;
                    pq.push(temp);
                }
                break;
            }
            case 6:
            {
                write_log(log, top);
                network.push_back(top);
                networkjobs++;
                max_queue(network, stats, 3);
                if (device[3] == 0)
                {
                    auto temp = event_finish(network, device, top, parameters, 3, networktime, networkresponsetime, networkqueuetime);
                    pq.push(temp);
                    get<1>(temp) = 8;
                    pq.push(temp);
                }
                break;
            }
            case 7:
            {
                write_log(log, top);
                device[3] = 0;
                if (!network.empty())
                {
                    auto temp = event_finish(network, device, top, parameters, 3, networktime, networkresponsetime, networkqueuetime);
                    pq.push(temp);
                    get<1>(temp) = 8;
                    pq.push(temp);
                }
            }
            break;
            //cpu arrival event
            case 8:
                write_log(log, top);
                cpu.push_back(top);
                cpujobs++;
                max_queue(cpu, stats, 0);
                if (device[0] == 0)
                {
                    auto temp = event_finish(cpu, device, top, parameters, 0, cputime, cpuresponsetime, cpuqueuetime);
                    pq.push(temp);
                }
                break;
            case 9:
                break;
            case 10:
                break;
            }
        }
        log << "Sim Finished at " << parameters["FINISH"] << endl;
        log.close();
        // test_function(max_queue);
        // I kind of just wanted to get the stats written so didn't have an elegant solution other than passing into a function of a huge int array that was a bit dangreous without knowing the numbers
        stats["cpu_throughput"] = (float)cpujobs / parameters["FINISH"];
        stats["disk1_throughput"] = (float)disk1jobs / parameters["FINISH"];
        stats["disk2_throughput"] = (float)disk2jobs / parameters["FINISH"];
        stats["network_throughput"] = (float)networkjobs / parameters["FINISH"];
        stats["cpu_utilization"] = (float)cputime / parameters["FINISH"];
        stats["disk1_utilization"] = (float)disk1time / parameters["FINISH"];
        stats["disk2_utilization"] = (float)disk2time / parameters["FINISH"];
        stats["network_utilization"] = (float)networktime / parameters["FINISH"];
        stats["cpu_avg_rt"] = (float)cputime / cpujobs;
        stats["disk1_avg_rt"] = (float)disk1time / disk1jobs;
        stats["disk2_avg_rt"] = (float)disk2time / disk2jobs;
        stats["cpu_max_rt"] = cpuresponsetime;
        stats["disk1_max_rt"] = disk1responsetime;
        stats["disk2_max_rt"] = disk2responsetime;
        stats["network_max_rt"] = networkresponsetime;
        stats["cpu_avg_queue"] = (float)cpuqueuetime / parameters["FINISH"];
        stats["disk1_avg_queue"] = (float)disk1queuetime / parameters["FINISH"];
        stats["disk2_avg_queue"] = (float)disk2queuetime / parameters["FINISH"];
        stats["network_avg_queue"] = (float)networkqueuetime / parameters["FINISH"];
        stats["pq_avg_queue"] = (float)pqtime / parameters["FINISH"];
        ofstream statfile("wow.csv", ios_base::app);
        if (statfile.is_open())
        {
            for (auto elem : stats)
            {
                cout << elem.first << ": " << elem.second << endl;
                statfile << elem.first << ", " << elem.second << endl;
            }
        }
        else
        {
            cout << "wow.csv could not be written to" << endl;
        }
        statfile << "\n";
        statfile.close();
    }
}

int makejob(int arrivemin, int arrivemax)
{
    int random = rand() % (arrivemax - arrivemin) + arrivemin;
    return random;
}

//reads in from config file in NAME NUMBER type format
map<string, int> configuration(ifstream &filename)
{
    string key;
    int value;
    map<string, int> temp;
    while (filename >> key >> value)
    {
        temp[key] = value;
    }
    return temp;
}

//generates a device finished event
tuple<int, int, string> event_finish(deque<tuple<int, int, string>> &device, int device_list[], tuple<int, int, string> event, map<string, int> parameters,
                                     int device_num, int &device_time, int &response_time, unsigned int &queuetime)
{
    device_list[device_num] = 1;
    size_t queuesize = device.size();
    auto temp = device.front();
    device.pop_front();

    switch (device_num)
    {
    case 0:
    {
        get<0>(temp) = get<0>(event) + rand() % (parameters["CPU_MAX"] - parameters["CPU_MIN"]) + parameters["CPU_MIN"];
        get<1>(temp) = 1;
        break;
    }
    case 1:
    {
        get<0>(temp) = get<0>(event) + rand() % (parameters["DISK1_MAX"] - parameters["DISK1_MIN"]) + parameters["DISK1_MIN"];
        get<1>(temp) = 3;
        break;
    }
    case 2:
    {
        get<0>(temp) = get<0>(event) + rand() % (parameters["DISK2_MAX"] - parameters["DISK2_MIN"]) + parameters["DISK2_MIN"];
        get<1>(temp) = 5;
        break;
    }
    case 3:
    {
        get<0>(temp) = get<0>(event) + rand() % (parameters["NETWORK_MAX"] - parameters["NETWORK_MIN"]) + parameters["NETWORK_MIN"];
        get<1>(temp) = 7;
        break;
    }
    }
    int duration = get<0>(temp) - get<0>(event);
    if (get<0>(temp) < parameters["FINISH"])
    {
        device_time += duration;
    }
    else{
        device_time = parameters["FINISH"];
    }
    if (duration > response_time)
    {
        response_time = duration;
    }
    queuetime += queuesize * response_time;
    return temp;
}

//this is keep track of max queue size
void max_queue(deque<tuple<int, int, string>> device, map<string, double> &stats, int device_num)
{
    switch (device_num)
    {
    case 0:
    {
        if (device.size() > stats["cpu_max_queue"])
        {
            stats["cpu_max_queue"] = device.size();
        }
        break;
    }
    case 1:
    {
        if (device.size() > stats["disk1_max_queue"])
        {
            stats["disk1_max_queue"] = device.size();
        }
        break;
    }
    case 2:
    {
        if (device.size() > stats["disk2_max_queue"])
        {
            stats["disk2_max_queue"] = device.size();
        }
        break;
    }
    case 3:
    {
        if (device.size() > stats["network_max_queue"])
        {
            stats["network_max_queue"] = device.size();
        }
        break;
    }
    }
}

//prints out events as they are processed on the queue
void write_log(ofstream &filename, tuple<int, int, string> event)
{
    switch (get<1>(event))
    {
    case 0:
        filename << "[Event] JOB ARRIVAL " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 1:
        filename << "[Event] CPU FINISH of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 2:
        filename << "[Event] DISK1 ARRIVAL of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 3:
        filename << "[Event] DISK1 FINISH of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 4:
        filename << "[Event] DISK2 ARRIVAL of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 5:
        filename << "[Event] DISK2 FINISH of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 6:
        filename << "[Event] NETWORK ARRIVAL of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 7:
        filename << "[Event] NETWORK FINISH of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 8:
        filename << "[Event] CPU ARRIVAL of " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 9:
        filename << "[Event] " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    case 10:
        filename << "[Event] " << get<2>(event) << " at time: " << get<0>(event) << endl;
        break;
    }
}

void test_function(void (*f)(deque<tuple<int, int, string>>, map<string, double> &, int))
{
    deque<tuple<int, int, string>> queuetemp;
    queuetemp.push_front(make_tuple(15, 5, "Job#10"));
    queuetemp.push_front(make_tuple(15, 5, "Job#10"));
    queuetemp.push_front(make_tuple(25, 5, "Job#10"));
    queuetemp.push_front(make_tuple(15, 5, "Job#10"));
    queuetemp.push_front(make_tuple(15, 5, "Job#10"));
    queuetemp.push_front(make_tuple(15, 5, "Job#10"));
    map<string, double> tempmap;
    int tempint = 0;

    f(queuetemp, tempmap, tempint);

    cout << tempmap["cpu_max_queue"] << " should be 6." << endl;
}