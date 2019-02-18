// ITEC371
// Project 3
// Christopher Blankenship
// Design - data structure for filesystem is a string that is built as user makes files/directories and is stored in binary file.
//        - specific design for project 3 - two deques to hold programs in the simulation and finished programs, i chose deques
//          as they seemed to be easier to work with than queues, queues are built upon deques.
// RU File System. This program mimics a filesystem that holds programs, text files, and directories.
// RU Shell - Builds onto the RU File System. Includes commands to be run (pwd,ls,cd <file>, cd .., mkdir <file>, cat <file>,
//            createTextFile <file>, run <file>, step <file>, start <file>)
// RATCH - Batch simulator where you run programs in a round robin queue sequence.

#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <queue>


using namespace std;

// Basic program struct, holds all necessary data for programs.
struct program
{
    string name;
    int timeReq; // time required to run
    int memReq; // memory space the program takes up
    int IOtime; // time to process io
    int amountIO; // time to wait for io
    bool IOflag; // is the program waiting for io?
    int total; // total time it has run (this is not stored in RUFS)
};

// struct for finished program.
struct fProgram
{
    program p; // finsihed program
    int finishedAt; // time the program finished at
};

void readText(char* file,int fileSize, int start, int numIn);
int readProgram(char* file, int start, int numIn);
string int_to_str(int num);

// getDirSize -> When the pointer is sitting at the front of a directory and you need to know the size
//               call this method. even works with directories within directories by recursion
// Parameters: dirPos -> points to the beginning of the directory for which we want the size
int getDirSize(int dirPos,char* file)
{
    ifstream ifile; //file stream
    ifile.open(file, ios::binary | ios::in); //open file at arg[1], in - only take input from it
    ifile.seekg (dirPos, ios::beg);

    //
    int dSize;
    ifile >> dSize;
    dirPos = dirPos + int_to_str(dSize).length();
    ifile.seekg (dirPos, ios::beg);


    for(int i = 0;i<dSize;i++)
    {
        string fileName;
        for(int i =0;i<10;i++)
        {
            char nameX;
            ifile >> nameX;
            fileName = fileName + nameX;
        }
        dirPos = dirPos +9;

        ifile.seekg (dirPos, ios::beg); //add pos to ls pointer
        char exten;
        ifile >> exten;
        dirPos = dirPos + 2;
        ifile.seekg (dirPos, ios::beg); //add pos to ls pointer

        if(exten=='t')
        {
            // Find out the contents to determine text file size
            int numContent;
            ifile >> numContent;
            dirPos = dirPos + int_to_str(numContent).length() + numContent;
        }
        if(exten=='p')
        {
            // Find out the CPU and memory requirements to determine program size
            int cpuReq;
            ifile >> cpuReq;
            dirPos = dirPos + int_to_str(cpuReq).length() + 1; // plus one for null seperating cpu and mem
            ifile.seekg(dirPos, ios::beg);
            int memReq;
            ifile >> memReq;
            dirPos = dirPos + int_to_str(memReq).length();
            ifile.seekg(dirPos, ios::beg);
            int iotime;
            ifile >> iotime;
            dirPos = dirPos + int_to_str(iotime).length();
            ifile.seekg(dirPos, ios::beg);
            int amountio;
            ifile >> amountio;
            dirPos = dirPos + int_to_str(amountio).length();
            ifile.seekg(dirPos, ios::beg);
        }
        if(exten=='d')
        {
            dirPos = getDirSize(dirPos,file);
            ifile.seekg(dirPos, ios::beg);
        }
    }
    return dirPos+11;
}

// testRoot -> This method is to test and see if our current directory is the root directory.
//             Mainly a debug method but it is called when the program is first run to verify
//             we start in root
// Parameters: string name -> this is the name of the current working directory
string testRoot(string name)
{
    string test; // this will be the first 6 characters of our current working directory

    // This loop finds those first 6 characters.
    for(int i =0;i<6;i++)
    {
        char testX;
        testX = name[i];
        test = test + testX;
    }

    // Compares to see if root.d is our current dir, if it is change working directory from root.d to root
    // followed by four null characters
    if(test=="root.d")
    {
        name = "root" + '\0' + '\0' + '\0' + '\0';
    }

    return name;
}

// getWD -> Returns the initial working directory
// Parameters: int pos -> Where we are in the binary file system
//             char* file -> The name of the file to check, this is always the file that contains our binary file system
//             We open it in main and need to pass it here, decided against the global file variable and opted for this parameter
string getWD(int pos,char* file)
{
    ifstream ifile; //file stream
    ifile.open(file, ios::binary | ios::in); //opens file
    ifile.seekg (pos, ios::beg);
    string workingD; // This will hold our current dir

    // Get dir name and then check it to see if it is root
    for(int i =0;i<8;i++)
    {
        char nameX;
        ifile >> nameX;
        workingD = workingD + nameX;
    }
    workingD = testRoot(workingD);

    return workingD;
}

// int_to_str -> Returns a string of the integer provided
// Parameters: num -> The integer to be converted.
string int_to_str(int num)
{
    stringstream ss;

    // Input the integer into a string stream and return the string of the string stream
    ss << num;
    return ss.str();
}

// readDir -> When using -r flag, print out the found directory
// Parameters: char* file -> The name of the file to check, this is always the file that contains our binary file system
//             int start -> Position of the start of the directory we are reading in the binary file system
//             int numIn -> number of indentations, if this directory is within another it will be 4*(number of parents)
int readDir(char* file, int start, int numIn)
{
    ifstream ifile; //file stream
    ifile.open(file, ios::binary | ios::in); //opens file
    ifile.seekg((start)*sizeof(char), ios::beg); // go to start of directory
    string directoryName;
    string ins; // string for indentation

    // This builds the indentation string
    for(int i =0;i<numIn;i++)
    {
        ins = ins + " ";
    }
    cout << ins << "Beginning list for ";

    // add indents for the current directory
    numIn = numIn + 4;

    // get directory name and print it out
    for(int i =0;i<8;i++)
    {
            char nameX;
            ifile >> nameX;
            directoryName = directoryName + nameX;
            cout << nameX;
    }
    cout << endl;

    // move past the directory name, get the number of files in the directory and adjust the position pointer
    ifile.seekg((start+11)*sizeof(char), ios::beg);
    int numfiles;
    ifile >> numfiles;
    int dirSize; // actual of directory including files and sub directories
    start = start + 12;

    // get the size of all files and sub files in dir
    for(int i = 0; i < numfiles; i++)
    {
        // look at extension of next directory/file and determine how to read it
        ifile.seekg((start+9)*sizeof(char), ios::beg);
        char exten;
        ifile >> exten;

        int fileSize;
        if(exten=='t')
        {
            ifile.seekg ((start+11)*sizeof(char), ios::beg);
            ifile >> fileSize;
            readText(file,fileSize,start, numIn);
        }
        if(exten=='p')
        {
            fileSize = readProgram(file,start,numIn);
        }
        if(exten=='d')
        {
            fileSize = readDir(file,start,numIn);
        }
        dirSize = dirSize + fileSize;
        start = start + fileSize + 12;
    }

    // At the end of the directory so back the indents up and rebuild ins
    numIn = numIn - 4;
    ins = "";
    for(int i =0;i<numIn;i++)
    {
        ins = ins + " ";
    }

    cout << ins << "Ending listing for " << directoryName << endl;

    return dirSize;
}

// readText -> When using -r flag, print out the found text file
// Parameters: char* file -> The name of the file to check, this is always the file that contains our binary file system
//             int start -> Position of the start of the text file we are reading in the binary file system
//             int numIn -> number of indentations, if this file is within a directory it will be 4*(number of parents)
void readText(char* file,int fileSize, int start, int numIn)
{
    ifstream ifile; //file stream
    ifile.open(file, ios::binary | ios::in); // opens file
    ifile.seekg((start)*sizeof(char), ios::beg); // go to beginning of text file
    string ins; // string for indentation

    // This builds the indentation string
    for(int i =0;i<numIn;i++)
    {
        ins = ins + " ";
    }
    cout << ins;

    // get file name and print it out
    for(int i =0;i<10;i++ )
    {
            char nameX;
            ifile >> nameX;
            cout << nameX;
    }
    cout << endl << "    " << ins;

    // get the contents of the file and print them out
    ifile.seekg((start+11+int_to_str(fileSize).length())*sizeof(char), ios::beg);
    for(int i =0;i<fileSize;i++ )
    {
            char contX;
            ifile.get(contX);
            cout << contX;
    }
    cout << endl;
}

// readProgram -> When using -r flag, print out the found program
// Parameters: char* file -> The name of the file to check, this is always the file that contains our binary file system
//             int start -> Position of the start of the program we are reading in the binary file system
//             int numIn -> number of indentations, if this file is within a directory it will be 4*(number of parents)
int readProgram(char* file, int start, int numIn)
{
    int CPU;
    int mem;
    int fileSize;

    ifstream ifile; //file stream
    ifile.open(file, ios::binary | ios::in); // opens file
    ifile.seekg((start)*sizeof(char), ios::beg); // go to the beginning of the program
    string ins; // string to use for indentation

    // This builds the indentation string
    for(int i =0;i<numIn;i++)
    {
        ins = ins + " ";
    }
    cout << ins;

    // get program name
    for(int i =0;i<10;i++ )
    {
            char nameX;
            ifile >> nameX;
            cout << nameX;
    }
    cout << endl;

    // Get CPU and mem requirements then print them on the screen
    ifile.seekg((start+11)*sizeof(char), ios::beg);
    ifile >> CPU;
    ifile.seekg((start+12+int_to_str(CPU).length())*sizeof(char), ios::beg);
    ifile >> mem;
    cout << ins << "    CPU Requirement: " << CPU << endl;
    cout << ins << "    Mem Requirement: " << mem << endl;

    // update fileSize and return it
    fileSize = int_to_str(CPU).length() + int_to_str(mem).length();
    return fileSize;
}

// createFile -> When using -c flag, create a text file or program
string createFile()
{
    // Get the file name and extension
    string name;
    string resultFile = ""; // full file to build
    do{
        cout << "Enter file name" << endl;
        cin >> name;
    }while (name.length() > 10);

    // separate name into extension and file name
    int nameLen = name.length();
    string fileName;
    char extension = name[nameLen-1];
    for(int i = 0;i<nameLen-2;i++  )
    {
        fileName = fileName + name[i];
    }

    // adding null terminators so get file name to 8 chars.
    if(fileName.length() < 8)
    {
        for (int i = nameLen-2;i<8;i++)
        {
            fileName = fileName + '\0';
        }
    }

    // grab extension and see if text file, if it is prompt for the contents of text file
    if(extension=='t')
    {
        string contents;
        cout << "Enter contents" << endl;
        cin.ignore();
        getline(cin, contents);
        resultFile = fileName + "." + extension + '\0' + int_to_str(contents.length()) + contents;
    }
    // grab extension and see if program
    else if(extension=='p')
    {

        cout << "Enter CPU requirements" << endl;
        string cpuReq;
        cin >> cpuReq;

        cout << "Enter memory requirements" << endl;
        string memReq;
        cin >> memReq;
        resultFile = fileName + "." + extension + '\0' + cpuReq + '\0' + memReq;
    }

    return resultFile;
}

// createDir -> When using -c flag, create a directory
string createDir()
{
    //get appropriate directory name
    string name;
    do
    {
        cout << "Enter directory name" << endl;
        cin >> name;
    }while(name.length() > 8);

    //adding null terminators so get dir name to 8 chars.
    if(name.length() < 8)
    {
        for (int i = name.length();i<8;i++)
        {
            name = name + '\0';
        }
    }

    int numFiles = 0;
    string dirList = ""; // contents of directory
    string command; // input to build directory

    //build dir, code is almost the same as building root but slight alterations
    while (command != "EndDir")
        {
            cout << "Command>";
            cin >> command;
            if(command=="CreateDir")
            {
                //recursion for directories inside directories
                string newDir = createDir();
                dirList += newDir;
                numFiles += 1;
            }
            else if(command=="CreateFile")
            {
                string newFile = createFile();
                dirList += newFile;
                numFiles +=1;
            }
            else if(command=="EndDir")
            {
                //Ends Dir, there is no code here. This is only here because it would print out invalid command and then end dir
                //this else if removes the invalid command printout
            }
            else
           {
              cout << "invalid command" << endl;
        }
    }

    string resultDir = name + ".d" + '\0' + int_to_str(numFiles) + dirList + "End" + name;
    return resultDir;
}

// main -> main method, builds the filesystem and shell to manipulate files and directories
// Parameters: argc -> number of arguments passed in
//             argv -> array of all the arguments
int main(int argc, char** argv)
{
    // using flags for -r or -c --- Project 1
    if(argc == 3)
    {
        // if first flag is -r then read
        // this is not working, had a lot of trouble reading from file.
        if (argv[1][1] == 'r')
        {
            int length;
            char * buffer;

            //open file at what preceeds the -r flag
            ifstream ifile; //file stream
            ifile.open(argv[2], ios::binary | ios::in); //open file at arg[2], in - only take input from it

            // find length of file system
            ifile.seekg (0, ios::end);
            length = ifile.tellg();
            ifile.seekg (0, ios::beg);

            // read in the filesystem into buffer
            buffer = new char [length];
            ifile.read(buffer, length*sizeof(char) );

            // print root.d
            for(int i = 0; i < 6; i++)
            {
                cout << buffer[i];
            }
            cout << ":" << endl;

            // move pointer and get number of files in root directory
            ifile.seekg (6*sizeof(char), ios::beg);
            int numfiles;
            ifile >> numfiles;

            // adjust for fileSize length
            int start = 6 + int_to_str(numfiles).length();

            // get files in root directory and then call the read methods based on type of file
            for(int i = 0; i < numfiles; i++)
            {
                // Get extension of next file
                ifile.seekg((start+9)*sizeof(char), ios::beg);
                char exten;
                ifile >> exten;

                int fileSize;
                int numIn = 0; // indentation in root directory is 0

                // Based on file type read that file
                if(exten=='t')
                {
                    ifile.seekg ((start+11)*sizeof(char), ios::beg);
                    ifile >> fileSize;
                    readText(argv[2],fileSize,start,numIn);
                }
                if(exten=='p')
                {
                    fileSize = readProgram(argv[2],start,numIn);
                }
                if(exten=='d')
                {
                    fileSize = readDir(argv[2],start,numIn);
                }
                start = start + fileSize + 12;
            }
            ifile.close(); // close file
        }

        // create for -c flag
        else if (argv[1][1] == 'c')
        {
            string rootFiles = ""; // build the root dir contents
            int rootNum = 0; // holds the number of files, will increment with each file we make in the root dir

            string command = "";
            cout << "CreateDir or CreateFile or EndDir Quit" << endl;

            // takes user input
            while (command != "Quit")
            {
                cout << "Command>";
                cin >> command;

                // Create a directory by calling createDir method and increment rootNum and build root contents with newly made directory
                if(command=="CreateDir")
                {
                    string newDir = createDir();
                    rootFiles += newDir;
                    rootNum += 1;
                }
                // Create a file by calling createFile method and increment rootNum and build root contents with newly made file
                else if(command=="CreateFile")
                {
                    string newFile = createFile();
                    rootFiles += newFile;
                    rootNum += 1;
                }
                // Quit, writes the file system to where ever we are storing it upon quitting
                else if(command=="Quit")
                {
                    //writes to files passed in when quitting
                    fstream ofile ;
                    ofile.open (argv[2], ios::binary    | ios::out);
                    string rootResult = "root.d" + int_to_str(rootNum) + rootFiles + "Endroot" + '\0';
                    ofile.write(rootResult.c_str(),rootResult.size());
                    ofile.close();
                }
                else
                {
                    cout << "invalid command" << endl;
                }
            }
        }
        else
        {
            cout << "invalid command" << endl;
        }
    }
    // no flags, just passing in a file ----- Project 2
    // RU SHELL and file system
    else if(argc == 2)
    {
        string command = "";
        ifstream ifile; //file stream
        ifile.open(argv[1], ios::binary | ios::in); //open file

        // test to see if the file exists
        if (!ifile.is_open())
        {
            std::ofstream outfile (argv[1]);
            outfile.close();
            ifile.open(argv[1], ios::binary | ios::in); //open file at arg[1], in - only take input from it
        }

        //if file is empty make root dir.
        ifile.seekg (0, ios::end);
        int initLength = ifile.tellg();
        string rootDir = "root.d0Endroot" + '\0';
        if(initLength==0)
        {
            fstream ofile ;
            ofile.open (argv[1], ios::binary    | ios::out);
            ofile.write(rootDir.c_str(),rootDir.size());
            ofile.close();
        }

        // Go to the beginning and get the root directory as the working directory and update
        // pos to reflect we are at the end of root's name and extension in the file system
        ifile.seekg (0, ios::beg);
        int pos=0;
        string workingD; // current working directory
        workingD = getWD(pos,argv[1]);
        pos = workingD.length()+2;//for name + extension
        int memSize = 0; // size of memory allocation in simulation
        int burstSize = 1; // default to actually progress if burst time was forgetten to be set.
        deque <program> sim; // deque for simulator
        deque <fProgram> finished; // deque for finished programs
        int curTime =0; // current time in simulation
        int RRpos =0; // position in simulator deque
        int memoryTaken = 0; // memory taken up within the simulation

        // This stack keeps track of positions for traversing down and up directories, used in cd mainly
        stack <int> dirPosits;
        dirPosits.push(pos);

        // takes user input
        while (command != "quit")
        {
            cout << "EnterCommand>";
            cin >> command;

            // Command is set memory
            // sets the memory for the program to use
            if(command == "setMemory")
            {
                cin >> memSize;
            }

            // Command is set burst
            // sets the number of units for each "burst cycle"
            else if(command == "setBurst")
            {
                cin >> burstSize;
            }

            // Command is get memory
            // prints memory size
            else if(command == "getMemory")
            {
                cout << "Amount of memory: " << memSize << endl;
            }

            // Command is add program
            // adds program to the RUFS, RUFS has been updated for IO time.
            else if(command == "addProgram")
            {
                program p; // program to add to RUFS
                string line; // program contents
                getline(cin, line);
                stringstream stream(line);

                // initialize program fields.
                while (!stream.eof())
                {
                    p.amountIO = 0;
                    p.IOtime = 0;
                    p.IOflag = false;
                    stream >> p.name;
                    stream >> p.timeReq;
                    stream >> p.memReq;
                    stream >> p.IOtime;
                    stream >> p.amountIO;
                }

                int ctfPos = pos; // position in RUFS where we are at for each step of the adding process
                int wdSize; // working directory size
                int length; // length of current file system
                string ending; // everything after the working directory size

                // find length of file system
                ifile.seekg(0, ios::end);
                length = ifile.tellg();
                ifile.seekg (ctfPos, ios::beg);

                // find working directory size
                ifile >> wdSize;

                // build newFS which will be the new file system with the new program
                // we take everything up until the working directory size and store that since it remains unchanged
                string newFS;
                ifile.seekg (0, ios::beg);
                for(int i = 0; i < ctfPos; i++)
                {
                    char x;
                    ifile >> x;
                    newFS = newFS + x;
                }

                // we update workind directory size and store it into newFS
                // and move our pointer up to where the new program will be made
                newFS = newFS + int_to_str(wdSize+1);
                ctfPos = ctfPos + int_to_str(wdSize+1).length();
                ifile.seekg (ctfPos, ios::beg);

                // save the second part of the file system
                // newFS + ending would be the old file system with the exception of working directory size
                for(int i = 0; i < (length - ctfPos); i++)
                {
                    char x;
                    ifile >> x;
                    ending = ending + x;
                }

                // create the text file name from what was passed into input
                string shortName = p.name;

                if(shortName.length() < 8)
                {
                    for (int i = shortName.length(); i < 8; i++)
                    {
                        shortName = shortName + '\0';
                    }
                }

                // add both parts of the new file system with the program in between
                newFS = newFS + shortName + ".p" + '\0' + int_to_str(p.timeReq) + '\0' + int_to_str(p.memReq) + '\0' + int_to_str(p.IOtime) + '\0' + int_to_str(p.amountIO) + ending;

                // Overwrite old file system with new file system
                fstream ofile;
                ofile.open (argv[1], ios::binary | ios::out);
                ofile.write(newFS.c_str(),newFS.size());
                ofile.close();

                ifile.seekg (pos, ios::beg);
            }

            // Command is run
            // advances through the simulation deque and finishes all jobs inside it.
            else if(command == "run")
            {
                cout << "Advancing the system until all jobs finished" << endl;

                while(!sim.empty())
                {
                    //output for status of simulation

                    cout << "Current time <" << curTime << ">" << endl;

                    cout << "Running job " << sim[RRpos].name << " has " << sim[RRpos].timeReq << " time left and is using "
                    << sim[RRpos].memReq << " memory resources." << endl;

                    // output of simulation queue, less the running job
                    cout << "The queue is:";
                    for(int job = 0; job < sim.size(); job++)
                    {
                        if(!sim[job].IOflag && job != RRpos)
                        {
                            int position = job;
                            if(job < RRpos)
                                position = position + 1;
                            cout << endl << "    Position " << position << ": job " << sim[job].name << " has " << sim[job].timeReq << " units left and is using "
                            << sim[job].memReq << " memory resources." << endl;
                        }
                        if(sim.size() <= 1)
                        {
                            cout << " empty" << endl;
                        }
                    }

                    // output of finished job queue
                    if(!finished.empty())
                    {
                        cout << "Finished jobs are:" << endl;
                        for(int job = 0; job < finished.size(); job++)
                        {
                            cout << "    " << finished[job].p.name << " " << finished[job].p.total << " " << finished[job].finishedAt << endl;
                        }
                    }

                    // output for programs doing IO
                    for(int job = 0; job < sim.size(); job++)
                    {
                        if(sim[job].IOflag && job != RRpos)
                        {
                            cout << "The process " << sim[job].name << " is obtaining IO and will be back in " << sim[job].amountIO << " unit." << endl;
                        }
                    }

                    // If the running job needs IO go do that
                    if(sim[RRpos].amountIO > 0)
                    {
                        curTime = curTime + sim[RRpos].IOtime;
                        sim[RRpos].timeReq = sim[RRpos].timeReq - sim[RRpos].IOtime;
                        sim[RRpos].IOflag = true;

                        // update jobs waiting on IO that is not the running job
                        for(int job = 0; job < sim.size(); job++)
                        {
                            if(sim[job].IOflag && job != RRpos)
                            {
                                sim[job].amountIO = sim[job].amountIO - sim[RRpos].IOtime;
                            }
                        }
                    }
                    // No IO, then run the program for the burst size of until finished, also update other programs waiting on IO
                    else
                    {
                        for(int i = 0; i < burstSize; i++)
                        {
                            if(sim[RRpos].timeReq > 0)
                            {
                                curTime = curTime + 1;
                                sim[RRpos].timeReq = sim[RRpos].timeReq - 1;
                                for(int job = 0; job < sim.size(); job++)
                                {
                                    if(sim[job].IOflag && job != RRpos)
                                    {
                                        sim[job].amountIO = sim[job].amountIO - 1;
                                    }
                                }
                            }
                        }

                        // If finished add it to the finished program deque and remove it from simulation
                        if(sim[RRpos].timeReq <= 0)
                        {
                            fProgram fp;
                            fp.p = sim[RRpos];
                            fp.finishedAt = curTime;
                            finished.push_back(fp), sim.erase(sim.begin()+RRpos);
                            RRpos = RRpos - 1;
                        }
                    }

                    // update IO flags for any programs that finished IO this iteration
                    for(int job = 0; job < sim.size(); job++)
                    {
                        if(sim[job].amountIO <= 0)
                        {
                            sim[job].IOflag = false;
                        }
                    }

                    // move to next job
                    RRpos = RRpos + 1;

                    // If you hit the end of the queue then move to the beginning.
                    if(RRpos >= sim.size())
                    {
                        RRpos = 0;
                    }
                }

                // final output for simulation
                cout << "Current time <" << curTime << ">" << endl;
                cout << "Running job is empty" << endl << "The queue is: empty" << endl;
                cout << "Finished jobs are:" << endl;
                        for(int job = 0; job < finished.size(); job++)
                        {
                            cout << "    " << finished[job].p.name << " " << finished[job].p.total << " " << finished[job].finishedAt << " " << endl;
                        }
            }

            // Command is step
            // steps through the simulation for a certain number of units.
            else if(command == "step")
            {
                int stepUnits; // units to step
                cin >> stepUnits;
                cout << "Advancing the system " << int_to_str(stepUnits) << " units" << endl;

                // find out what the system time should be when the "step" command finishes, that is the conditional for our loop.
                int endingTime = curTime + stepUnits;
                while(curTime <= endingTime)
                {


                    // output for simulation
                    cout << "Current time <" << curTime << ">" << endl;

                    cout << "Running job " << sim[RRpos].name << " has " << sim[RRpos].timeReq << " time left and is using "
                    << sim[RRpos].memReq << " memory resources." << endl;

                    // output for simulation queue, less the running job
                    cout << "The queue is:";
                    for(int job = 0; job < sim.size(); job++)
                    {
                        if(!sim[job].IOflag && job != RRpos)
                        {
                            int position = job;
                            if(job < RRpos)
                            {
                                position = position + 1;
                            }
                            cout << endl << "    Position " << position << ": job " << sim[job].name << " has " << sim[job].timeReq << " units left and is using "
                            << sim[job].memReq << " memory resources." << endl;
                        }
                        if(sim.size() <= 1)
                        {
                            cout << " empty" << endl;
                        }
                    }

                    // output for finished program deque
                    if(!finished.empty())
                    {
                        cout << "Finished jobs are:" << endl;
                        for(int job = 0; job < finished.size(); job++)
                        {
                            cout << "    " << finished[job].p.name << " " << finished[job].p.total << " " << finished[job].finishedAt << endl;
                        }
                    }

                    // output for programs performing IO
                    for(int job = 0; job < sim.size(); job++)
                    {
                        if(sim[job].IOflag && job != RRpos)
                        {
                            cout << "The process " << sim[job].name << " is obtaining IO and will be back in " << sim[job].amountIO << " unit." << endl;
                        }
                    }

                    // If running program needs to do IO
                    if(sim[RRpos].amountIO > 0)
                    {
                        curTime = curTime + sim[RRpos].IOtime;
                        sim[RRpos].timeReq = sim[RRpos].timeReq - sim[RRpos].IOtime;
                        sim[RRpos].IOflag = true;

                        // update other programs doing IO
                        for(int job = 0; job < sim.size(); job++)
                        {
                            if(sim[job].IOflag && job != RRpos)
                            {
                                sim[job].amountIO = sim[job].amountIO - sim[RRpos].IOtime;
                            }
                        }
                    }
                    else // If it just needs to run non-IO processes
                    {
                        for(int i = 0; i < burstSize; i++ )
                        {
                            if(sim[RRpos].timeReq > 0)
                            {
                                curTime = curTime + 1;
                                sim[RRpos].timeReq = sim[RRpos].timeReq - 1;
                                for(int job = 0; job < sim.size(); job++)
                                {
                                    if(sim[job].IOflag && job != RRpos)
                                    {
                                        sim[job].amountIO = sim[job].amountIO - 1;
                                    }
                                }
                            }
                        }

                        // add finished programs to deque, remove from simulation
                        if(sim[RRpos].timeReq <= 0)
                        {
                            fProgram fp;
                            fp.p = sim[RRpos];
                            fp.finishedAt = curTime;
                            finished.push_back(fp), sim.erase(sim.begin()+RRpos);
                            RRpos = RRpos - 1;
                        }
                    }

                    for(int job = 0; job < sim.size(); job++)
                    {
                        if(sim[job].amountIO <= 0)
                        {
                            sim[job].IOflag = false;
                        }
                    }

                    // update position in queue
                    RRpos = RRpos + 1;

                    // If we hit the end of queue wrap around to front.
                    if(RRpos >= sim.size())
                    {
                        RRpos = 0;
                    }
                }

            }

            // Command is start
            // adds program from RUFS to simulation deque
            else if(command == "start")
            {
                string progName;
                cin >> progName;

                // Take the input and add null characters to it as well as save short name for printing later
                string shortName = ""; // used in printing later
                for(int i = 0; i < progName.length(); i++)
                {
                    shortName = shortName + progName[i];
                }

                if(shortName.length() < 8)
                {
                    for (int i = shortName.length(); i < 8; i++)
                    {
                        shortName = shortName + '\0';
                    }
                }
                progName = shortName + ".p";

                // get files in working directory, we need to check each file for target, then update pointer
                int startPos = pos;
                ifile.seekg(pos, ios::beg);
                int numFiles;
                ifile >> numFiles;
                startPos = startPos + int_to_str(numFiles).length();
                ifile.seekg(startPos, ios::beg);

                int skipName = 11; // used to skip over name and extension when moving poiners in following loop

                // for each file in our working directory check and see if it the target text file
                for(int i = 0; i < numFiles; i++)
                {
                    // get file name of the next file in our working directory
                    string checkName;
                    for(int j = 0; j < 10; j++)
                    {
                        char next;
                        ifile >> next;
                        checkName = checkName + next;
                    }

                    // we found the target file, now print its content out
                    if(checkName == progName)
                    {
                        program p;
                        // initialize fields for program p
                        startPos = startPos + skipName;
                        ifile.seekg(startPos, ios::beg);
                        p.name = shortName;
                        ifile >> p.timeReq;
                        startPos = startPos + int_to_str(p.timeReq).length() + 1; // plus one for null seperating cpu and mem
                        ifile.seekg(startPos, ios::beg);
                        ifile >> p.memReq;
                        startPos = startPos + int_to_str(p.memReq).length() + 1; // plus one for null seperating cpu and mem
                        ifile.seekg(startPos, ios::beg);
                        ifile >> p.IOtime;
                        startPos = startPos + int_to_str(p.IOtime).length() + 1; // plus one for null seperating cpu and mem
                        ifile.seekg(startPos, ios::beg);
                        ifile >> p.amountIO;
                        ifile.seekg(pos, ios::beg);
                        p.IOflag = false;
                        p.total = p.timeReq;

                        memoryTaken = memoryTaken + p.memReq;

                        //check memory of simulation
                        if(memoryTaken <= memSize)
                            sim.push_back(p); // enough space, so add the program to sim
                        else // there is not enough space, through a exception statement
                        {
                            cout << "Not enough memory allocated" << endl;
                            memoryTaken = memoryTaken - p.memReq;
                        }
                    }
                    // file we are at is not target file, find next file
                    else
                    {
                        // update pointer to find the type of file, this will help determine file size
                        startPos = startPos + 9;
                        ifile.seekg(startPos, ios::beg);
                        char exten;
                        ifile >> exten;
                        startPos = startPos + 2;
                        ifile.seekg (startPos, ios::beg);

                        // based on file, move positioning accordingly
                        if(exten == 't')
                        {
                            // move positioning based on content size for text files
                            int numContent;
                            ifile >> numContent;
                            startPos = startPos + int_to_str(numContent).length() + numContent;
                        }
                        else if(exten == 'd')
                        {
                            startPos = getDirSize(startPos,argv[1]);
                        }
                        else if(exten == 'p')
                        {
                            // get cpu and memory requirements to determine program size
                            int cpuReq;
                            ifile >> cpuReq;
                            startPos = startPos + int_to_str(cpuReq).length() + 1; // plus one for null seperating cpu and mem
                            ifile.seekg(startPos, ios::beg);
                            int memReq;
                            ifile >> memReq;
                            startPos = startPos + int_to_str(memReq).length()+1;
                            ifile.seekg(startPos, ios::beg);
                            int iotime;
                            ifile >> iotime;
                            startPos = startPos + int_to_str(iotime).length()+1;
                            ifile.seekg(startPos, ios::beg);
                            int amountio;
                            ifile >> amountio;
                            startPos = startPos + int_to_str(amountio).length();
                        }
                        ifile.seekg(startPos, ios::beg);
                    }
                }

            }

            // Command is pwd
            // just prints out the working directory. As we move directories we update workingD in those
            // so there is no calculations here as we keep track of workingD along the way
            else if(command == "pwd")
            {
                cout << "Current directory is " << workingD << endl;
            }

            // Command is ls
            // traverse through working directory and print out its contents
            else if(command == "ls")
            {
                cout << "Direcory name: ";

                // derive current directory from working directory
                string curDir = "";
                int upPos = 0;
                // Find the position of the last '/'
                for(int i = 0; i < workingD.length(); i++)
                {
                    if(workingD[i] == '/')
                    {
                        upPos = i+1;
                    }
                }
                // Grab everything after the last '/'
                for(int i = upPos;i<workingD.length();i++)
                {
                    curDir=curDir+workingD[i];
                }
                cout << curDir << endl;

                int lsPos=pos; // pointer for ls command
                // Grab number of files in working directory and update lsPos to first file
                ifile.seekg (lsPos, ios::beg);
                int numFiles;
                ifile >> numFiles;
                lsPos = lsPos + int_to_str(numFiles).length();
                ifile.seekg (lsPos, ios::beg);

                // Print each file name and type for working directory
                for (int i = 0; i < numFiles; i++)
                {
                    // Get file name and extension
                    string fileName;
                    for(int i =0;i<10;i++)
                    {
                        char nameX;
                        ifile >> nameX;
                        fileName = fileName + nameX;
                    }
                    cout << "Filename:  " << fileName;

                    // grab file extension and update lsPos
                    lsPos = lsPos + 9;
                    ifile.seekg (lsPos, ios::beg);
                    char exten;
                    ifile >> exten;
                    lsPos = lsPos + 2;
                    ifile.seekg (lsPos, ios::beg);

                    // Print out file type based on extension and update lsPos to end of that file
                    if(exten=='t')
                    {
                        cout << " Type:  Text file" << endl;
                        // Find out the contents to determine text file size
                        int numContent;
                        ifile >> numContent;
                        lsPos = lsPos + int_to_str(numContent).length() + numContent;
                    }
                    else if(exten=='d')
                    {
                        cout << " Type:  Directory" << endl;
                        // Find out the size of each file in the directory to determine directory size
                        lsPos = getDirSize(lsPos,argv[1]);
                    }
                    else if(exten=='p')
                    {
                        cout << " Type:  Program" << endl;
                        // Find out the CPU and memory requirements to determine program size
                        int cpuReq;
                        ifile >> cpuReq;
                        lsPos = lsPos + int_to_str(cpuReq).length() + 1; // plus one for null seperating cpu and mem
                        ifile.seekg(lsPos, ios::beg);
                        int memReq;
                        ifile >> memReq;
                        lsPos = lsPos + int_to_str(memReq).length();
                    }
                    ifile.seekg (lsPos, ios::beg);
                }
                ifile.seekg (pos, ios::beg);
            }

            // Command is cd
            // if a directory name is passed in - find target directory and change working directory to it
            // or for cd .. back up one directory
            else if(command=="cd")
            {
                string dirName;
                cin >> dirName;

                // command is not cd ..
                if(dirName!="..")
                {
                // command is cd .. so we need to back up a directory
                    string shortName = dirName; // This will be updated to workingD, here before we add rest of directory name formating

                    // Build rest of directory name
                    if(dirName.length() < 8)
                    {
                        for (int i = dirName.length();i<8;i++)
                        {
                            dirName = dirName + '\0';
                        }
                    }
                    dirName = dirName + ".d";

                    // move past the working directory file size
                    int cdPos = pos;
                    ifile.seekg(pos, ios::beg);
                    int numFiles;
                    ifile >> numFiles;
                    cdPos = cdPos + int_to_str(numFiles).length();
                    ifile.seekg(cdPos, ios::beg);

                    int skipName=2; // for skipping extensions later

                    // traverse through working directory to see if we can find target directory
                    for(int i = 0;i<numFiles;i++)
                    {
                        ifile.seekg(cdPos, ios::beg);
                        string checkName=""; // This will be the name of each file to check against target directory

                        // Get file name
                        for(int j = 0; j < 10; j++)
                        {
                            ifile.seekg(cdPos, ios::beg);

                            char next;
                            ifile >> next;
                            checkName = checkName + next;
                            cdPos = cdPos + 1;

                        }

                        // check file name to target
                        if(checkName==dirName)
                        {
                            // we found the target directory so update our overall positioning
                            // then push our new position onto the stack and update working directory
                            pos = cdPos+skipName-1;//to negate last update in filename finder loop
                            dirPosits.push(pos);
                            workingD = workingD + "/" + shortName;
                            ifile.seekg(pos, ios::beg);
                        }
                        else
                        {
                            // File is not the target directory so move cd position to next file
                            cdPos = cdPos - 1; // to negate last update in previous loop
                            ifile.seekg(cdPos, ios::beg);

                            // find current file size based on type of file and adjust pointer
                            char exten;
                            ifile >> exten;
                            cdPos = cdPos + 2;
                            ifile.seekg(cdPos, ios::beg);

                            if(exten=='t')
                            {
                                // Find out the contents to determine text file size
                                int numContent;
                                ifile >> numContent;
                                cdPos = cdPos + int_to_str(numContent).length() + numContent;
                            }
                            else if(exten=='d')
                            {
                                cdPos = getDirSize(cdPos,argv[1]);
                            }
                            else if(exten=='p')
                            {
                                // Find out the CPU and memory requirements to determine program size
                                int cpuReq;
                                ifile >> cpuReq;
                                cdPos = cdPos + int_to_str(cpuReq).length() + 1; // plus one for null seperating cpu and mem
                                ifile.seekg(cdPos, ios::beg);
                                int memReq;
                                ifile >> memReq;
                                cdPos = cdPos + int_to_str(memReq).length();
                            }
                            ifile.seekg (cdPos, ios::beg);
                        }
                    }
                }
                else if (dirName=="..")
                {
                    // verify we are not in root directory, cant move up past it
                    if(dirPosits.top()!=6)
                    {
                        // move up in the stack
                        dirPosits.pop();

                        // adjust the positioning to reflect the pop
                        pos = dirPosits.top();
                        ifile.seekg(pos, ios::beg);

                        // find last find last '/' and update positioning to right before it
                        string temp = "";
                        int cdPos = 0;
                        for(int i = 0;i<workingD.length();i++)
                        {
                            if(workingD[i]=='/')
                            {
                                cdPos = i;
                            }
                        }
                        // workingD everything right before last '/'
                        // basically remove the last '/' and last directory from working directory
                        for(int i = 0;i<cdPos;i++)
                        {
                            temp=temp+workingD[i];
                        }
                        workingD = temp;
                    }
                }
            }

            // Command is mkdir
            // create a directory in current directory
            else if(command=="mkdir")
            {
                string dirName;
                cin >> dirName;
                int mkPos = pos;
                int wdSize; // we need to increment our working directory size to account for this new directory
                int length;
                string ending; // this will be the length of the binary file system at the start of this method

                // find the length of binary file system
                ifile.seekg (0, ios::end);
                length = ifile.tellg();
                ifile.seekg (mkPos, ios::beg);

                // get working directory size
                ifile >> wdSize;

                string newFS; // this will be everything in the file system up to the point where we will add the new directory

                // build the new file system up to the working directory size
                ifile.seekg (0, ios::beg);
                for(int i =0;i<mkPos;i++)
                {
                    char x;
                    ifile >> x;
                    newFS = newFS + x;
                }

                // Update the working directory size, add it to the file system and update the positioning
                newFS = newFS + int_to_str(wdSize+1);
                mkPos = mkPos + int_to_str(wdSize+1).length();
                ifile.seekg (mkPos, ios::beg);

                // Build ending, everything after working directory size
                // our filesystem is now split between newFS and ending (with newFS have the increment of the wdSize)
                for(int i =0;i<(length - mkPos);i++)
                {
                    char x;
                    ifile >> x;
                    ending = ending + x;
                }

                // create the directory name from input
                if(dirName.length() < 8)
                {
                    for (int i = dirName.length();i<8;i++)
                    {
                        dirName = dirName + '\0';
                    }
                }

                // update newFS to reflect the beginning portion, then the new directory with nothing in it, then the ending
                newFS = newFS + dirName + ".d" + '\0' + "0End" + dirName + ending;

                // write over top of the old file.
                fstream ofile ;
                ofile.open (argv[1], ios::binary    | ios::out);
                ofile.write(newFS.c_str(),newFS.size());
                ofile.close();

            }

            // Command is cat
            // this finds the target text file and prints out its name and contents
            else if(command=="cat")
            {
                string catName;
                cin >> catName;

                // Take the input and add null characters to it as well as save short name for printing later
                string shortName=""; // used in printing later
                for(int i =0;i<catName.length()-2;i++)
                {
                    shortName = shortName + catName[i];
                }

                if(shortName.length() < 8)
                {
                    for (int i = shortName.length();i<8;i++)
                    {
                        shortName = shortName + '\0';
                    }
                }
                catName = shortName + ".t";

                // get files in working directory, we need to check each file for target, then update pointer
                int catPos = pos;
                ifile.seekg(pos, ios::beg);
                int numFiles;
                ifile >> numFiles;
                catPos = catPos + int_to_str(numFiles).length();
                ifile.seekg(catPos, ios::beg);

                int skipName=11; // used to skip over name and extension when moving poiners in following loop

                // for each file in our working directory check and see if it the target text file
                for(int i = 0;i<numFiles;i++)
                {
                    // get file name of the next file in our working directory
                    string checkName;
                    for(int j = 0; j < 10; j++)
                    {
                        char next;
                        ifile >> next;
                        checkName = checkName + next;
                    }

                    // we found the target file, now print its content out
                    if(checkName==catName)
                    {
                        cout << "Text file contents: " << endl;

                        //find its content size
                        catPos = catPos + skipName;
                        ifile.seekg(catPos, ios::beg);
                        int conSize;
                        ifile >> conSize;

                        // print out its contents
                        for(int x = 0; x < conSize;x++)
                        {
                            char z;
                            ifile.get(z);
                            cout<<z;
                        }
                        cout << endl;
                        ifile.seekg(pos, ios::beg);
                    }
                    // file we are at is not target file, find next file
                    else
                    {
                        // update pointer to find the type of file, this will help determine file size
                        catPos = catPos + 9;
                        ifile.seekg(catPos, ios::beg);
                        char exten;
                        ifile >> exten;
                        catPos = catPos + 2;
                        ifile.seekg (catPos, ios::beg);

                        // based on file, move positioning accordingly
                        if(exten=='t')
                        {
                            // move positioning based on content size for text files
                            int numContent;
                            ifile >> numContent;
                            catPos = catPos + int_to_str(numContent).length() + numContent;
                        }
                        else if(exten=='d')
                        {
                            catPos = getDirSize(catPos,argv[1]);
                        }
                        else if(exten=='p')
                        {
                            // get cpu and memory requirements to determine program size
                            int cpuReq;
                            ifile >> cpuReq;
                            catPos = catPos + int_to_str(cpuReq).length() + 1; // plus one for null seperating cpu and mem
                            ifile.seekg(catPos, ios::beg);
                            int memReq;
                            ifile >> memReq;
                            catPos = catPos + int_to_str(memReq).length();
                        }
                        ifile.seekg(catPos, ios::beg);
                    }
                }

            }

            // Command is createTexTFile
            // this creates a text file in our working directory
            else if(command=="createTextFile")
            {
                string textName;
                string contents;
                cout << "Enter filename>";
                cin >> textName;
                cout << "Enter file contents>";
                cin.ignore();
                getline(cin, contents);
                int ctfPos = pos;
                int wdSize; // working directory size
                int length; // length of current file system
                string ending; // everything after the working directory size

                // find length of file system
                ifile.seekg(0, ios::end);
                length = ifile.tellg();
                ifile.seekg (ctfPos, ios::beg);

                // find working directory size
                ifile >> wdSize;

                // build newFS which will be the new file system with the new text file
                // we take everything up until the working directory size and store that since it remains unchanged
                string newFS;
                ifile.seekg (0, ios::beg);
                for(int i =0;i<ctfPos;i++)
                {
                    char x;
                    ifile >> x;
                    newFS = newFS + x;
                }

                // we update workind directory size and store it into newFS
                // and move our pointer up to where the new text file will be made
                newFS = newFS + int_to_str(wdSize+1);
                ctfPos = ctfPos + int_to_str(wdSize+1).length();
                ifile.seekg (ctfPos, ios::beg);

                // save the second part of the file system
                // newFS + ending would be the old file system with the exception of working directory size
                for(int i =0;i<(length - ctfPos);i++)
                {
                    char x;
                    ifile >> x;
                    ending = ending + x;
                }

                // create the text file name from what was passed into input
                string shortName="";
                for(int i =0;i<textName.length()-2;i++)
                {
                    shortName = shortName + textName[i];
                }
                if(shortName.length() < 8)
                {
                    for (int i = shortName.length();i<8;i++)
                    {
                        shortName = shortName + '\0';
                    }
                }

                // add both parts of the new file system with the text file in between
                newFS = newFS + shortName + ".t" + '\0' + int_to_str(contents.length()) + contents + ending;

                // Overwrite old file system with new file system
                fstream ofile ;
                ofile.open (argv[1], ios::binary    | ios::out);
                ofile.write(newFS.c_str(),newFS.size());
                ofile.close();

                ifile.seekg (pos, ios::beg);


            }

            // Command is quit
            // Shell quits
            else if(command=="quit")
            {
                // do nothing but quit on next loop iteration.
            }
            else
            {
                cout << "invalid command" << endl;
            }
        }
    }

    return 0;
}
