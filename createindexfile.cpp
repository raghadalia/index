#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
using namespace std;

struct IndexEntry {
    string address;
    vector<int> locations;
};


struct StudentRecord {
    int stdno;
    string stdname;
    string address;
};
struct StudentInfo {
    int stdno;      
    string stdname; 
    string address; 
    int location;   
    int recordLength; 

   
};

void createIndexFromFile(const string& dataFileName, const string& indexFileName) {
    ifstream dataFile(dataFileName);
    ofstream indexFile(indexFileName);

    if (!dataFile || !indexFile) {
        cerr << "Failed to open input or output file." << endl;
        return;
    }

    map<string, IndexEntry> index;

    string line;
    int location = 0;

    while (getline(dataFile, line)) {
        istringstream iss(line);
        string stdno, stdname, address;
        iss >> stdno >> stdname >> address;

        if (!iss.fail()) {
           
            if (index.find(address) == index.end()) {
                
                IndexEntry newIndexEntry;
                newIndexEntry.address = address;
                index[address] = newIndexEntry;
            }

            
            index[address].locations.push_back(location);
        }

        location = dataFile.tellg(); 
    }

    dataFile.close();

    for (const auto& entry : index) {
        indexFile << entry.first << " ";
        for (size_t i = 0; i < entry.second.locations.size(); ++i) {
            indexFile << entry.second.locations[i];
            if (i < entry.second.locations.size() - 1) {
                indexFile << ", ";
            }
        }
        indexFile << endl;
    }

    indexFile.close();
}

void insertRecord(const string& dataFileName, const string& indexFileName, const StudentRecord& student) {
   
    fstream dataFile(dataFileName, ios::in | ios::out );

    if (!dataFile) {
        cerr << "Failed to open the data file for writing." << endl;
        return;
    }

    ifstream dataCheckFile(dataFileName);
    string line;
    while (getline(dataCheckFile, line)) {
        istringstream iss(line);
        string token;
        iss >> token; 
        if (token == to_string(student.stdno)) {
            dataCheckFile.close();
            cout << "Student number already exists. Please choose a unique student number." << endl;
            dataFile.close(); 
            return;
        }
    }
    dataCheckFile.close();

    int location = dataFile.tellg();
    dataFile << student.stdno << " " << student.stdname << " " << student.address << endl;

    dataFile.close(); 
    ofstream indexFile(indexFileName, ios::in);

    if (!indexFile) {
        cerr << "Failed to open the index file for writing." << endl;
        return;
    }

    
    indexFile << student.address << ": " << location << endl;

    indexFile.close(); 

    createIndexFromFile(dataFileName, indexFileName);
}
vector<StudentInfo> getStdnoAndLocation(const string& dataFileName, const string& indexFileName) {
    ifstream indexFile(indexFileName);

    if (!indexFile) {
        cerr << "Failed to open the index file for reading." << endl;
        return {};
    }

    vector<StudentInfo> stdnoAndLocation;

    StudentInfo info;
    string line;

    while (getline(indexFile, line)) {
        istringstream iss(line);
        iss >> info.address;
        char comma;
        while (iss >> info.location) {
            ifstream dataFile(dataFileName);
            dataFile.seekg(info.location);
            dataFile >> info.stdno >> info.stdname; 

            
            int newRecordLength = to_string(info.stdno).length() + info.stdname.length() + info.address.length() + 2; // 2 for spaces
            info.recordLength = newRecordLength;

            dataFile.close();
            stdnoAndLocation.push_back(info);

           
            iss >> comma;
        }
    }

    indexFile.close();

    return stdnoAndLocation;
}


void deleteStudentRecord(const string& dataFileName, const string& indexFileName, int stdnoToDelete, const string& addressToDelete) {
    vector<StudentInfo> stdnoLocationAddress = getStdnoAndLocation(dataFileName, indexFileName);

    bool found = false;

    ofstream tempDataFile("temp_data.txt");

    if (!tempDataFile) {
        cerr << "Failed to create a temporary data file." << endl;
        return;
    }
    
    for (const StudentInfo& info : stdnoLocationAddress) {
        if (info.address == addressToDelete && info.stdno == stdnoToDelete) {
        

            found = true;
        }
        else {
            
            ifstream dataFile(dataFileName);
            dataFile.seekg(info.location);
            string record;
            getline(dataFile, record);
            if (!dataFile.eof()) {
                tempDataFile << record << endl;
            }
            dataFile.close();
        }
    }

    tempDataFile.close();

    if (!found) {
        cout << "No student records found with student number: " << stdnoToDelete << " and address: " << addressToDelete << endl;
        remove("temp_data.txt");
        return;
    }

    ofstream dataFile(dataFileName, ios::trunc);

    if (!dataFile) {
        cerr << "Failed to open the data file for writing." << endl;
        return;
    }

    ifstream tempDataFileRead("temp_data.txt");

    if (!tempDataFileRead) {
        cerr << "Failed to open the temporary data file for reading." << endl;
        return;
    }

    string line;
    while (getline(tempDataFileRead, line)) {
        dataFile << line << endl;
    }

    dataFile.close();
    tempDataFileRead.close();

    remove("temp_data.txt");

    createIndexFromFile(dataFileName, indexFileName);
}



void updateStudentRecord(const string& dataFileName, const string& indexFileName, string addressBeforeUpdate, int stdnoToUpdate, const StudentRecord& updatedStudent) {
    
    vector<StudentInfo> stdnoAndLocation = getStdnoAndLocation(dataFileName, indexFileName);

    for (StudentInfo& info : stdnoAndLocation) {
        if (info.stdno == stdnoToUpdate && info.address == addressBeforeUpdate) {
            
            int newRecordLength = to_string(updatedStudent.stdno).length() + updatedStudent.stdname.length() + updatedStudent.address.length() + 3; // 3 for spaces and newline

            
            fstream dataFile(dataFileName, ios::in | ios::out);

            if (!dataFile) {
                cerr << "Failed to open the data file for writing." << endl;
                return;
            }

           
            dataFile.seekp(info.location, ios::beg);

            
            dataFile << updatedStudent.stdno << " " << updatedStudent.stdname << " " << updatedStudent.address << endl;

            
            int deltaLength = newRecordLength - info.recordLength;
            if (deltaLength != 0) {
                for (StudentInfo& nextInfo : stdnoAndLocation) {
                    if (nextInfo.location > info.location) {
                        nextInfo.location += deltaLength;
                    }
                }
            }

            
            info.recordLength = newRecordLength;

            
            dataFile.close();

            
            createIndexFromFile(dataFileName, indexFileName);
            cout << "Student record updated successfully." << endl;
            return;
        }
    }

    cout << "No student records found with student number: " << stdnoToUpdate << " and address: " << addressBeforeUpdate << endl;
}

void updateStudentName(const string& dataFileName, const string& indexFileName, int stdnoToUpdate, const string& addressToUpdate, const string& newStdName) {
   
    vector<StudentInfo> stdnoAndLocation = getStdnoAndLocation(dataFileName, indexFileName);

    for (StudentInfo& info : stdnoAndLocation) {
        if (info.stdno == stdnoToUpdate && info.address == addressToUpdate) {
            
            int newRecordLength = to_string(stdnoToUpdate).length() + newStdName.length() + info.address.length() ; 

            
            fstream dataFile(dataFileName, ios::in | ios::out);

            if (!dataFile) {
                cerr << "Failed to open the data file for writing." << endl;
                return;
            }

            
            dataFile.seekp(info.location, ios::beg);

            
            dataFile << stdnoToUpdate << " " << newStdName << " " << info.address << endl;

           
            int deltaLength = newRecordLength - info.recordLength;
            if (deltaLength != 0) {
                for (StudentInfo& nextInfo : stdnoAndLocation) {
                    if (nextInfo.location > info.location) {
                        nextInfo.location += deltaLength;
                    }
                }
            }

            
            info.recordLength = newRecordLength;

            dataFile.close();

            
            createIndexFromFile(dataFileName, indexFileName);
            cout << "Student name updated successfully." << endl;
            return;
        }
    }

    cout << "No student records found with student number: " << stdnoToUpdate << " and address: " << addressToUpdate << endl;
}

void updateStudentAddress(const string& dataFileName, const string& indexFileName, int studentNumber, const string& updatedAddress) {
   
    vector<StudentInfo> stdnoAndLocation = getStdnoAndLocation(dataFileName, indexFileName);

    for (StudentInfo& info : stdnoAndLocation) {
        if (info.stdno == studentNumber) {
            
            int newAddressLength = info.address.length();

            
            string paddedUpdatedAddress = updatedAddress + string(newAddressLength - updatedAddress.length()+2, ' ');

            
            fstream dataFile(dataFileName, ios::in | ios::out);

            if (!dataFile) {
                cerr << "Failed to open the data file for writing." << endl;
                return;
            }

           
            dataFile.seekp(info.location, ios::beg);

            dataFile << studentNumber << " " << info.stdname << " " << paddedUpdatedAddress << endl;

           
            dataFile.close();

            
            createIndexFromFile(dataFileName, indexFileName);
            cout << "Student address updated successfully." << endl;
            return;
        }
    }

    cout << "No student records found with student number: " << studentNumber << endl;
}


void displayStudentsByAddress(const string& dataFileName, const string& indexFileName, const string& targetAddress) {
    ifstream indexFile(indexFileName);

    if (!indexFile) {
        cerr << "Failed to open the index file for reading." << endl;
        return;
    }

    vector<StudentInfo> stdnoAndLocation;

    StudentInfo info;
    string line;
    bool found = false; 

    while (getline(indexFile, line)) {
        istringstream iss(line);
        iss >> info.address;
        char comma;
        while (iss >> info.location) {
            ifstream dataFile(dataFileName);
            dataFile.seekg(info.location);
            dataFile >> info.stdno >> info.stdname >> info.address;

            
            iss >> comma;

            if (info.address == targetAddress) {
                
                cout << "Student Number: " << info.stdno << endl;
                cout << "Student Name: " << info.stdname << endl;
                cout << "Address: " << info.address << endl;
                cout << endl;
                found = true; 
            }

            dataFile.close();
        }
    }

    indexFile.close();

    if (!found) {
        cout << "No student records found for address: " << targetAddress << endl;
    }
}

int main() {
    string dataFileName;
    cout << "Enter the name of the data file without double cotation: ";//recomended to use the name of data file i created in the project folder (data.txt)
    cin >> dataFileName;
    string indexFileName = "index.txt";
    fstream dataFile(dataFileName, ios::in | ios::out );

    if (!dataFile) {
        cerr << "Failed to open the data file for reading and writing." << endl;
        return 1;
    }

    ofstream indexFile("index.txt");

    if (!indexFile) {
        cerr << "Failed to open the index file for writing." << endl;
        return 1;
    }
    createIndexFromFile(dataFileName, indexFileName);
    map<string, IndexEntry> index;

    
    
  /*  vector<StudentInfo> stdnoLocationAddress = getStdnoAndLocation(dataFileName, indexFileName);

    for (const StudentInfo& info : stdnoLocationAddress) {
        cout << "Student Number: " << info.stdno << endl;
        cout << "Location in Data File: " << info.location << endl;
        cout << "Address: " << info.address << endl;
        cout << endl;
    }
    */
    int choice;
    do {
        cout << "1. Display students information by address" << endl;
        cout << "2. Insert a student record" << endl;
        cout << "3. Delete a student record" << endl;
        cout << "4. Update a student record" << endl;
        cout << "5. Update student name" << endl;
        cout << "6. Update student address" << endl;
        cout << "7. Exit" << endl;
        cout << "Enter your choice: ";
       
        cin >> choice;

        switch (choice) {
        case 1: {
            string  targetAddress;
            cout << "Enter the student/s address that you want to display thier info. : ";
            cin >> targetAddress;
            displayStudentsByAddress(dataFileName, indexFileName, targetAddress);
            break;
        }
        case 2: {
            StudentRecord newStudent;

            cout << "Enter student number: ";
            cin >> newStudent.stdno;
            cin.ignore(); // Ignore the newline 
            cout << "Enter student name: ";
            getline(cin, newStudent.stdname);
            cout << "Enter student address: ";
            getline(cin, newStudent.address);

           insertRecord("data.txt", "index.txt", newStudent);
            break;
        }
        case 3: {
            createIndexFromFile(dataFileName, indexFileName);
            int studentNumber;
            string studentAddress;
            cout << "Enter student number to delete: ";
            cin >> studentNumber;
            cin.ignore(); 
            cout << "Enter student address to delete: ";
            getline(cin, studentAddress);

            deleteStudentRecord(dataFileName, indexFileName, studentNumber, studentAddress);
            break;
        }
        case 4: {
            createIndexFromFile(dataFileName, indexFileName);
            int studentNumber;
            StudentRecord updatedRecord;
            string addressBeforeUpdate;
            cout << "Enter student number to update: ";
            cin >> studentNumber;

            cin.ignore();

            cout << "Enter student address : ";
            getline(cin, addressBeforeUpdate);
            

            cout << "Enter updated student name: ";
            getline(cin, updatedRecord.stdname);

            cout << "Enter updated student address: ";
            getline(cin, updatedRecord.address);
            updatedRecord.stdno = studentNumber;
            updateStudentRecord(dataFileName, indexFileName, addressBeforeUpdate, studentNumber, updatedRecord);
            cout << "Updated student number: " << updatedRecord.stdno << endl;

            break;


        }
        case 5:
        {
            
            int studentNumber;
            string updatedName, studentAddress;

            cout << "Enter student number to update name: ";
            cin >> studentNumber;
            cin.ignore(); 

            cout << "Enter student address to update name: ";
            getline(cin, studentAddress);

            cout << "Enter updated student name: ";
            getline(cin, updatedName);

           
            updateStudentName(dataFileName, indexFileName, studentNumber, studentAddress, updatedName);


            break;
        }
        case 6:
        {
           
            int studentNumber;
            string  studentAddress,updatedAddress;

            cout << "Enter student number to update address: ";
            cin >> studentNumber;
            cin.ignore(); 

            cout << "Enter student address to update address: ";
            getline(cin, studentAddress);

            cout << "Enter updated student address: ";
            getline(cin, updatedAddress);

          
            updateStudentAddress(dataFileName, indexFileName, studentNumber, updatedAddress);


            break;
        }
        case 7:
            cout << "Exiting the program." << endl;
            break;
        default:
            cout << "Invalid choice. Please try again." << endl;
            break;
        }
    } while (choice != 7);


    return 0;
}
