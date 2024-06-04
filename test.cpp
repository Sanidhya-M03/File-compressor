#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <codecvt>
#include <locale>
#include <memory>
#include<vector>
#include<queue>

using namespace std;

struct HuffmanNode {
    wchar_t character;
    int frequency;
    shared_ptr<HuffmanNode> left;
    shared_ptr<HuffmanNode> right;

    HuffmanNode(wchar_t ch, int freq) : character(ch), frequency(freq), left(nullptr), right(nullptr) {}

    // Compare nodes for priority queue
    bool operator>(const HuffmanNode& other) const {
        return frequency > other.frequency;
    }
};
struct Compare {
    bool operator()(shared_ptr<HuffmanNode> const& lhs, shared_ptr<HuffmanNode> const& rhs) {
        return lhs->frequency > rhs->frequency;
    }
};
void generateCodes(shared_ptr<HuffmanNode> root, const wstring& str, unordered_map<wchar_t, wstring>& huffmanCode) {
    if (!root) return;

    // Found a leaf node
    if (!root->left && !root->right) {
        huffmanCode[root->character] = str;
    }

    generateCodes(root->left, str + L"0", huffmanCode);
    generateCodes(root->right, str + L"1", huffmanCode);
}
int main(){
    wstring file_name;
    wcout<<L"Enter the Filename";
    wcin>>file_name;
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    string narrow_filename = converter.to_bytes(file_name);
    ifstream inputFile(narrow_filename, std::ios::binary);

    if (!inputFile.is_open()) {
        wcerr << L"Error opening file: " << file_name << endl;
        return 1;
    }
    string fileContent((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    
    inputFile.close();
    wstring wFileContent = converter.from_bytes(fileContent);

    unordered_map<wchar_t, int> frequencyTable;
    for (wchar_t ch : wFileContent) {
        ++frequencyTable[ch];
    }
    priority_queue<shared_ptr<HuffmanNode>, vector<shared_ptr<HuffmanNode>>, Compare> pq;

    for (auto pair : frequencyTable) {
        pq.push(make_shared<HuffmanNode>(pair.first, pair.second));
    }
    while (pq.size() != 1) {
        // Remove the two nodes of highest priority (lowest frequency) from the queue
        shared_ptr<HuffmanNode> left = pq.top(); pq.pop();
        shared_ptr<HuffmanNode> right = pq.top(); pq.pop();

        // Create a new internal node with these two nodes as children and with frequency equal to the sum of the two nodes' frequencies.
        int sum = left->frequency + right->frequency;
        shared_ptr<HuffmanNode> newNode = make_shared<HuffmanNode>(L'\0', sum);
        newNode->left = left;
        newNode->right = right;

        // Add the new node to the priority queue
        pq.push(newNode);
    }

    // The remaining node is the root of the Huffman Tree
    shared_ptr<HuffmanNode> root = pq.top();

    // Traverse the Huffman Tree to generate codes for each character
    unordered_map<wchar_t, wstring> huffmanCode;
    generateCodes(root, L"", huffmanCode);

    // Write the compressed data to output.txt
    wofstream outputFile("output.txt", std::ios::binary);

    if (!outputFile.is_open()) {
        wcerr << L"Error opening output file: output.txt" << endl;
        return 1;
    }

    for (wchar_t ch : wFileContent) {
        outputFile << huffmanCode[ch];
    }

    outputFile.close();

    wcout << L"Compressed data written to output.txt" << endl;

    return 0;
}