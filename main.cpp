#include <bits/stdc++.h>
using namespace std;
class BTree {
private:
    // The file that holds the b-tree nodes.
    fstream file;
    // The number of values one node can hold.
    int m;
    // The maximum number of records the b-tree file can hold.
    int numberOfRecords;
    // The number of characters allocated for each pair in the b-tree file.
    int charSize;
public:
    // Constructor.
    BTree() {
        m=0;
        numberOfRecords=0;
        charSize=0;
    }
    // Returns the number of characters a record in the b-tree file takes
    // based on the specified pair size and number of values that one record can hold.
    int recordSize() const { return charSize + 2 * m * charSize; }
////////////////////////////////////////////////////////////////////////////////////////
    void CreateIndexFileFile (char* filename, int NumberOfRecords, int M) {
        this->m = M;
        this->numberOfRecords = NumberOfRecords;
        this->charSize = M;
        openFile(filename);
        Starter();
    }
////////////////////////////CreateIndexFileFile////////////////////////////////////////////////////////////////////
    void openFile(char* filename) {
        file.open(filename, ios::in | ios::out | ios::binary | ios::trunc);
    }
////////////////////////////////////////////////////////////////////////////////////////////////
    void DisplayIndexFileContent() {
        for (int i = 0; i < numberOfRecords; ++i) {
            char record[recordSize()];
            file.seekg(i * recordSize(),ios::beg);
            file.read(record, recordSize());
            for (int j = 0; j < recordSize(); ++j)
                cout << record[j];
            cout << endl;
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////
    ~BTree() {
        file.close();
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool record_valid(int recordNumber) const {
        // If the record number is not between 1 and numberOfRecords
        if (recordNumber <= 0 || recordNumber > numberOfRecords)
            return false;
        return true;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool pair_valid(int pairNumber) const {
        // If the pair number is not between 1 and m
        if (pairNumber <= 0 || pairNumber > m)
            return false;
        return true;
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read from record as pairs first is key and second is reference
    pair<int, int> read_pair(int recordNumber, int pairNumber) {

        if (record_valid(recordNumber) && pair_valid(pairNumber)) {

            // Create and return the cell
            pair<int, int> thePair;
            thePair.first = read_val(recordNumber, 2 * pairNumber - 1);
            thePair.second = read_val(recordNumber, 2 * pairNumber);

            return thePair;

        }
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//store all keys inside record
    vector<pair<int, int>> read_node_values(int recordNumber) {
        // Validate input
        if (record_valid(recordNumber)) {

            // Go to the specified record on the b-tree file
            file.seekg(recordNumber * recordSize() + charSize, ios::beg);

            // Create and return the node
            vector<pair<int, int>> theNode;

            // Read every pair in the node
            for (int i = 1; i <= m; ++i) {

                pair<int, int> p;

                p = read_pair(recordNumber, i);

                // If it is empty then the rest is empty, so return
                if (p.second == -1) return theNode;

                // Otherwise, continue reading the node
                theNode.push_back(p);
            }
            return theNode;
        }
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////
    int nextEmpty() {
        return read_val(0, 1);
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////
    int read_val(int rowIndex, int columnIndex) {
        // Go to the specified cell
        file.seekg(rowIndex * recordSize() + columnIndex * charSize, ios::beg);

        // Read and return the integer value in the cell
        char val[charSize];

        file.read(val, charSize);

        return stoi(string(val));
    }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int InsertNewRecordAtIndex(int recordId, int reference) {
        vector<pair<int, int>> current; //vector represent node

        // If the root is empty
        if (isEmpty(1)) {
            // Insert in root

            // The next empty for the record at next empty
            int nextEmptyNext = read_val(nextEmpty(), 1);

            // Update the next empty
            write_val(nextEmptyNext, 0, 1);

            // Create the read_node_values
            current = read_node_values(1);

            // Insert the new pair
            current.emplace_back(recordId, reference);

            // Sort the node
            sort(current.begin(), current.end());

            // Write the node in root
            writeNode(current, 1);

            // Mark the root as leaf
            leaf_state(1, 0);

            // Return the index of the record in which the insertion happened
            // i.e. the root in this case
            return 1;
        }
        // Keep track of visited records to updateAfterInsert them after insertion
        stack<int> visited;

        // Search for recordId in every node in the b-tree
        // starting with the root
        int i = 1;
        bool found;
        while (!isLeaf(i)) {
            visited.push(i);
            current = read_node_values(i);
            found = false;
            for (auto p: current) {
                // If a greater value is found
                if (p.first >= recordId) {
                    // B-Tree traversal
                    i = p.second;
                    found = true;
                    break;
                }
            }

            // B-Tree traversal
            if (!found) i = current.back().second;
        }
        current = read_node_values(i);
        // Insert the new pair
        current.emplace_back(recordId, reference);
        // Sort the node
        sort(current.begin(), current.end());
        int newFromSplitIndex = -1;
        // If record overflowed after insertion
        if (current.size() > m)
            newFromSplitIndex = split(i, current);
        else
            // Write the node in root
            writeNode(current, i);

        // If the insertion happened in root
        // Then there are no parents to updateAfterInsert
        if (i == 1) return i;

        // Otherwise, updateAfterInsert parents
        while (!visited.empty()) {
            int lastVisitedIndex = visited.top();
            visited.pop();
            newFromSplitIndex = updateAfterInsert(lastVisitedIndex, newFromSplitIndex);
        }
        // Return the index of the inserted record
        // or -1 if insertion failed
        return i;
    }

////////////////////////////////////////////////END OF INSERT///////////////////////////////////////////////////////////
    void Starter() {
        // For each record
        for (int recordIndex = 0; recordIndex < numberOfRecords ; ++recordIndex) {
            // Write -1 in the first cell to indicate it is
            // an empty record (available for allocation)
            write_val(-1, recordIndex, 0);

            // Write the number of the next empty record in the available list
            if (recordIndex == numberOfRecords - 1)
                write_val(-1, recordIndex, 1);
            else
                write_val(recordIndex + 1, recordIndex, 1);

            // Fill the rest of the record with -1s
            for (int cellIndex = 2; cellIndex <= m * 2; ++cellIndex)
                write_val(-1, recordIndex, cellIndex);
        }
    }

    string int_Converter(int value) const {
        stringstream result;

        // Convert the integer value into a string
        string stringValue = to_string(value);
        // Write the string
        result << stringValue;
        // Write spaces until the final result's size becomes the cell size
        for (int i = 0; i < charSize - stringValue.size(); ++i) result << ' ';
        return result.str();
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void write_val(int value, int rowIndex, int columnIndex) {
        file.seekg(rowIndex * recordSize() + columnIndex * charSize, ios::beg);
        file << int_Converter(value);
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool isLeaf(int recordNumber) {
        return read_val(recordNumber, 0) == 0;
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool isEmpty(int recordNumber) {
        return read_val(recordNumber, 0) == -1;
    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void writeNode(const vector<pair<int, int>> &read_node_values, int recordNumber) {
        clearRecord(recordNumber);
        file.seekg(recordNumber * recordSize() + charSize, ios::beg);
        for (auto p: read_node_values)
            file << int_Converter(p.first) << int_Converter(p.second);
        file.seekg(0, ios::beg);
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   Change Record status to leaf or not
    void leaf_state(int recordNumber, int leafStatus) {
        file.seekg(recordNumber * recordSize(), ios::beg);
        file << int_Converter(leafStatus);
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int updateAfterInsert(int parentRecordNumber, int newChildRecordNumber) {
        vector<pair<int, int>> newParent;
        auto parent = read_node_values(parentRecordNumber);
        // For each value in parent
        for (auto p: parent) {
            auto childNode = read_node_values(p.second);
            // Add the maximum of the value's child
            newParent.emplace_back(childNode.back().first, p.second);
        }
        // If there was a new child from previous split
        if (newChildRecordNumber != -1)
            //  Add the maximum of the new value's child
            newParent.emplace_back(read_node_values(newChildRecordNumber).back().first, newChildRecordNumber);

        sort(newParent.begin(), newParent.end());

        int newFromSplitIndex = -1;

        // If record overflowed after insertion
        if (newParent.size() > m)
            newFromSplitIndex = split(parentRecordNumber, newParent);
        else
            // Write new parent
            writeNode(newParent, parentRecordNumber);

        return newFromSplitIndex;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int split(int recordNumber, vector<pair<int, int>> originalNode) {
        if (recordNumber == 1)
            return split(originalNode);

        // Get the index of the new record created after split
        int newRecordNumber = nextEmpty();

        // If there are no empty records, then splitting fails
        if (newRecordNumber == -1) return -1;

        // Update the next empty cell with the next in available list
        write_val(read_val(newRecordNumber, 1), 0, 1);

        // Distribute originalNode on two new nodes
        vector<pair<int, int>> firstNode, secondNode;

        // Fill first and second nodes from originalNode
        auto middle(originalNode.begin() + (int) (originalNode.size()) / 2);
        for (auto it = originalNode.begin(); it != originalNode.end(); ++it) {
            if (distance(it, middle) > 0) firstNode.push_back(*it);
            else secondNode.push_back(*it);
        }
        // Clear originalNodeIndex and newNodeIndex
        clearRecord(recordNumber);
        clearRecord(newRecordNumber);

        leaf_state(recordNumber, 0);
        writeNode(firstNode, recordNumber);

        leaf_state(newRecordNumber, 0);
        writeNode(secondNode, newRecordNumber);

        return newRecordNumber;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void clearRecord(int recordNumber) {
        for (int i = 1; i <= m * 2; ++i){
            file.seekg(recordNumber * recordSize() + i * charSize, ios::beg);
            file << int_Converter(-1);
        }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool split(vector<pair<int, int>> root) {
        // Find 2 empty records for the new nodes
        int firstNodeIndex = nextEmpty();
        if (firstNodeIndex == -1) return false;
        // Get next empty node in available list
        int secondNodeIndex = read_val(firstNodeIndex, 1);
        if (secondNodeIndex == -1) return false;
        // Update the next empty cell with the next in available list
        write_val(read_val(secondNodeIndex, 1), 0, 1);
        vector<pair<int, int>> firstNode, secondNode;
        // Fill first and second nodes from root
        auto middle(root.begin() + (int) (root.size()) / 2);
        for (auto it = root.begin(); it != root.end(); ++it) {
            if (distance(it, middle) > 0) firstNode.push_back(*it);
            else secondNode.push_back(*it);
        }
        leaf_state(firstNodeIndex, leafStatus(1));
        writeNode(firstNode, firstNodeIndex);
        leaf_state(secondNodeIndex, leafStatus(1));
        writeNode(secondNode, secondNodeIndex);
        clearRecord(1);
        // Create new root with max values from the 2 new nodes
        vector<pair<int, int>> newRoot;
        newRoot.emplace_back(firstNode.back().first, firstNodeIndex);
        newRoot.emplace_back(secondNode.back().first, secondNodeIndex);
        markNonLeaf(1);
        writeNode(newRoot, 1);
        return true;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void markNonLeaf(int recordNumber) {
        file.seekg(recordNumber * recordSize(), std::ios::beg);
        file << int_Converter(1);
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int leafStatus(int recordNumber) {
        return read_val(recordNumber, 0);
    }

    
    // delete from non-leaf nodes
    void DeleteRecordFromIndex (int recordId, int reference){
        vector<pair<int, int>> current; //vector represent node

       
        // Keep track of visited records and its child to updateAfterDelete them after Deletetion
        stack<int> visited;
        stack<int> refChild;

        // Search for recordId in every node in the b-tree
        // starting with the root
        int i = 1;
        bool found = false;
        while (!found) {
            visited.push(i);
            current = read_node_values(i);
            for (auto p: current) {
                // If a greater value is found
                if (p.first >= recordId) {
                    if(p.second == reference){
                        found = true;
                    }
                    else{
                        // B-Tree traversal
                        i = p.second;
                        refChild.push(p.second);
                    }
                    
                    break;
                }
            }

        }

        visited.pop();
        current = read_node_values(i);


        

        //get the pair of the record to remove it 
        pair<int , int> value;
        value.first = recordId;
        value.second = reference;

        current.erase(remove(current.begin(), current.end(), value), current.end());

        // remove the record from the root
        if(visited.empty()){
            writeNode(current, i);
            return;
        }

        // case 1 and 2
        // if the node after deletion contains greater than or equal m/2
        if(current.size()>=(m/2)){
            writeNode(current, i);
            checkForUpdate(visited , refChild , recordId);
            
        }
        else{
            
            pair<int, bool> siblingAndPosition = getSibling(visited.top() , refChild.top());
            int siblingIndex = siblingAndPosition.first;
            bool isPrevious = siblingAndPosition.second;

            // get from sibling is allowed (case 3)
            if(getFromSibling(siblingIndex)){

                // get from the next sibling 
                // this does not affect on the value in the parent of the sibling
                if(!isPrevious){
                    

                }

                // get from the previous sibling
                // this affects on the value in the parent of the sibling
                else{
                    
                }


            }

            // merge two nodes(case 4)
            else{

               

            }

        }         
    }


    // delete from the leaf nodes
    void DeleteRecordFromIndex (int recordId){
        vector<pair<int, int>> current; //vector represent node

        // If the root is empty
        if (isEmpty(1)) {
            cout<< "not found"<< endl;
            return;
        }
        // if the id is greater than the largest value in the root
        // that mean it is not found
        vector<pair<int, int>> root = read_node_values(1);
        
        if(recordId > root.back().first) {
            cout<< "not found"<< endl;
            return;
        }
        
        // Keep track of visited records and its child to updateAfterDelete them after Deletetion
        stack<int> visited;
        stack<int> refChild;

        // Search for recordId in every node in the b-tree
        // starting with the root
        int i = 1;
        while (!isLeaf(i)) {
            visited.push(i);
            current = read_node_values(i);
            for (auto p: current) {
                // If a greater value is found
                if (p.first >= recordId) {
                    // B-Tree traversal
                    i = p.second;
                    refChild.push(p.second);
                    break;
                }
            }

        }
        current = read_node_values(i);


        // search in current node that may be deleted
        bool exist = false;
        int ref = 0;
        for (auto pair: current){
            if (pair.first == recordId){
                exist = true;
                ref = pair.second;
            }
        }

        // if is not exist 
        if(!exist){
            cout<< "not found"<< endl;
            return;
        }
        // record is exist

        //get the pair of the record to remove it 
        pair<int , int> value;
        value.first = recordId;
        value.second = ref;




        current.erase(remove(current.begin(), current.end(), value), current.end());

        // remove the record from the root
        if(visited.empty()){
            writeNode(current, i);
            if(current.size()==0){
                updateHeaderAfterDelete(1);
            }
            cout<<"delete successfully"<<endl;

            return;
        }

        // case 1 and 2
        // if the node after deletion contains greater than or equal m/2
        if(current.size()>=(m/2)){
            writeNode(current, i);
            checkForUpdate(visited , refChild , recordId);
            
        }
        else{
            
            pair<int, bool> siblingAndPosition = getSibling(visited.top() , refChild.top());
            int siblingIndex = siblingAndPosition.first;
            bool isPrevious = siblingAndPosition.second;

            // get from sibling is allowed (case 3)
            if(getFromSibling(siblingIndex)){

                // get from the next sibling 
                // this does not affect on the value in the parent of the sibling
                if(!isPrevious){
                    
                    // keep track of the previous max to update it after take key from next sibling
                    int previousMax = current.back().first;
                    

                    
                    checkForUpdate(visited , refChild , recordId , previousMax );

                }

                // get from the previous sibling
                // this affects on the value in the parent of the sibling
                else{

                    // previous max of the previous sibling
                    int previousMax ;

                    updatePreviousSibling(visited.top() , previousMax);

                    checkForUpdate(visited , refChild , recordId);

                }


            }

            // merge two nodes(case 4)
            else{

                // previous max of the sibling
                int previousMax ;
                

                // remove the pair of the deleted node from the parent
                bool isUpdated = updateParentAfterMerge(visited.top(), refChild.top());


                // if not update call the function checkForUpdate on the sibling
                if(!isUpdated){
                    
                // change the last reference of the deleted node 
                // to the reference of the merged sibling node
                refChild.pop();
                refChild.push(siblingIndex);


                checkForUpdate(visited , refChild , recordId , previousMax);

                }


                
                updateHeaderAfterDelete(i);

            }

        }


        updateRootAfterDelete();

        cout<<"delete successfully"<<endl;
                

    }


    // update the root if it contains one value
    // merge the child into it
    void updateRootAfterDelete(){}

    // remove the key and its reference of the deleted node from the parent
    bool updateParentAfterMerge(int parentRecordNumber,  int deleteRef){}


    // set the header to the new deleted node
    // and put the previous (next empty) in the deleted node
    void updateHeaderAfterDelete(int recordNumber){
        // get the next empty from the header
        int nextempty = nextEmpty();

        // Update the next empty with the deleted node
        write_val(recordNumber, 0, 1);

        // update the status of the node 
        leaf_state(recordNumber , -1);


        // write the next empty that we get it from the header 
        // in the deleted node

        vector<pair<int, int>> updatedNextEmpty;
        pair<int, int> newNextEmpty;

        // next empty
        newNextEmpty.first = nextempty;
        newNextEmpty.second = -1;
        updatedNextEmpty.push_back(newNextEmpty);
        writeNode(updatedNextEmpty , recordNumber);
    }

    //Search in parent to get the ref of prev max 
    //then take the max from the child and update prev max in the parent with the new max

    void updatePreviousSibling(int parentRecordNumber , int prevMax){}


    // check all visited node need to update or not 
    void checkForUpdate(stack<int> visited , stack<int> refChild ,int recordId , int previousMax = -1){
        // check in the visited node 
        // we remove pair exists in the parent then update parent(case 2)
        // else don't update parent (case 1)
        while (!visited.empty()) {
            int lastVisitedIndex = visited.top();
            visited.pop();
            int lastRefIndex = refChild.top();
            refChild.pop();

            // if the deleted record is in the parent then update parent
            // or the previous max equal value not -1 that mean there is new max so we must update the parent
            if(isExistInParent(lastVisitedIndex , recordId) || previousMax!=-1){
                updateAfterDelete(lastVisitedIndex , lastRefIndex , recordId , previousMax);
            }
            else{
                break;
            }
        }
    }



    //get sibling and its position (previous sibling or next sibling)
    // false if next sibling and true if previous sibling
    pair<int , bool> getSibling(int parentRecordNumber , int ref){}

    // return true if the taking value from the sibling is allowed otherwise return false
    bool getFromSibling(int siblingRecoredNumber){}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //return true if the deleted record in the parent otherwise return false
    bool isExistInParent(int parentRecordNumber , int recordId){
        vector<pair<int, int>> parent;
        parent = read_node_values(parentRecordNumber);
        for (auto pair: parent){
            if (pair.first == recordId){
                return true;
            }
        }

        return false;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // updated all node that affected by the deletion
    void updateAfterDelete(int parentRecordNumber , int ref , int recordId , int previousMax){
        // get parent and its child
        vector<pair<int, int>> parent;
        vector<pair<int, int>> child;
        parent = read_node_values(parentRecordNumber);
        child = read_node_values(ref);

        //get the max value in child
        int newMax = child.back().first;

        // search for the vaule that we remove it to change to new max value
        for (auto& pair : parent) {
            // if the deleted record is equal change it to the new max value
            // or if there is previous max in the parent change it to the new max value
            if (pair.first == recordId || pair.first == previousMax) {
                pair.first = newMax;
            }
        }

        // write it in index file after update
        writeNode(parent, parentRecordNumber);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int SearchARecord(int recordId) {
        if (isEmpty(1)) return -1;
        vector<pair<int, int>> current;
        // Search for recordId in every node in the b-tree
        // starting with the root
        int i = 1;
        bool found;
        while (!isLeaf(i)) {
            current = read_node_values(i);
            found = false;
            for (auto p: current) {
                // If a greater value is found
                if (p.first >= recordId) {
                    // B-Tree traversal
                    i = p.second;
                    found = true;
                    break;
                }
            }
            // B-Tree traversal
            if (!found) i = current.back().second;
        }

        current = read_node_values(i);
        for (auto pair: current)
            if (pair.first == recordId)
                return pair.second;

        return -1;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};
class Example{
public :
    void search(BTree &btree){
        int recID;
        cout<< "Enter record ID: \n";
        cin >> recID;
        cout << btree.SearchARecord(recID) << endl;
    }
    void insertions(BTree &btree) {
        cout << "*****************************************************\n";
        cout<<"(3, 12) (7, 24) (10, 48) (24, 60) (14, 72) Records are Inserted\n";
        btree.InsertNewRecordAtIndex(3, 12);
        btree.InsertNewRecordAtIndex(7, 24);
        btree.InsertNewRecordAtIndex(10, 48);
        btree.InsertNewRecordAtIndex(24, 60);
        btree.InsertNewRecordAtIndex(14, 72);
        btree.DisplayIndexFileContent();
        cout << "*****************************************************\n";
        cout<<"Record (19, 84) is inserted\n";
        btree.InsertNewRecordAtIndex(19, 84);
        btree.DisplayIndexFileContent();
        cout << "*****************************************************\n";
        cout<<"(30, 96) (15, 108) (1, 120) (5, 132) Records are Inserted\n";
        btree.InsertNewRecordAtIndex(30, 96);
        btree.InsertNewRecordAtIndex(15, 108);
        btree.InsertNewRecordAtIndex(1, 120);
        btree.InsertNewRecordAtIndex(5, 132);
        btree.DisplayIndexFileContent();
        cout << "*****************************************************\n";
        cout<<"Record (2, 144) is inserted\n";
        btree.InsertNewRecordAtIndex(2, 144);
        btree.DisplayIndexFileContent();
        cout << "*****************************************************\n";
        cout<<"(8, 156), (9, 168), (6, 180), (11, 192), (12, 204), (17, 216), (18, 228) Records are Inserted\n";
        btree.InsertNewRecordAtIndex(8, 156);
        btree.InsertNewRecordAtIndex(9, 168);
        btree.InsertNewRecordAtIndex(6, 180);
        btree.InsertNewRecordAtIndex(11, 192);
        btree.InsertNewRecordAtIndex(12, 204);
        btree.InsertNewRecordAtIndex(17, 216);
        btree.InsertNewRecordAtIndex(18, 228);
        btree.DisplayIndexFileContent();
        cout << "*****************************************************\n";
        cout<<"Record (32, 240) is inserted\n";
        btree.InsertNewRecordAtIndex(32, 240);
        btree.DisplayIndexFileContent();
    }
};
int main() {
    int Choice;
    BTree btree;
    cout <<"If You Want to Run Example In Assigment Press 1 or 2 For New Example==>";
    cin>>Choice;
    bool go =false;
    if (Choice==1) {
        //For Me Please Don't Delete This Code
        //btree.CreateIndexFileFile("C:\\Users\\dell\\CLionProjects\\untitled4\\index.txt", 10,5);
        btree.CreateIndexFileFile("index.txt", 10,5);
        Example example;
        example.insertions(btree);
        go=true;

    }
    if (Choice==2 || go){
        if(!go){
        char *FileName=new char[100];
        int RecordSize,M;
        cout <<"Enter The File Name==>" <<endl;
        cin>>FileName;
        cout<<" Enter The RecordSize==>"<<endl;
        cin>>RecordSize;
        cout<<" Enter M==>"<<endl;
        cin>>M;
        btree.CreateIndexFileFile(FileName, RecordSize,M);
        }
        while(true){
            int c;
            cout << "1-InsertNewRecordAtIndex\n2-delete\n3-search\n4-DisplayIndexFileContent\n5-exit\n";
            cin>>c;
            if(c == 1){
                int recID,ref;
                cout << "Enter record ID and reference: \n";
                cin >> recID >> ref;
                if (btree.InsertNewRecordAtIndex(recID, ref) == -1)
                    cout << "There is no place to insert the record\n";
                else
                    cout << "Record inserted successfully\n";
            }
            else if(c == 2){
                int recID;
                cout<< "Enter record ID: \n";
                cin >> recID;
                btree.DeleteRecordFromIndex(recID);
             }
            else if(c== 3){
                int recID;
                cout<< "Enter record ID: \n";
                cin >> recID;
                if (btree.SearchARecord(recID) == -1)
                    cout << "Record not found\n";
                else
                {
                    cout << "Record found at reference: " << btree.SearchARecord(recID) << endl;
                }
                cout << "*****************************************************\n";
            }
            else if(c==4){
                cout << "*****************************************************\n";
                btree.DisplayIndexFileContent();
                cout << "*****************************************************\n";
            }
            else
                break;
        }
    }
    else
    {
        cout<<"Wrong Choice"<<endl;
    }
    cout <<"Good Bye"<<endl;
    return 0;
}
