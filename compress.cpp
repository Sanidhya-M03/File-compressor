#include <fstream>
#include <iostream>
#include <unordered_map>
#include <string>
#include <codecvt>
#include <locale>
#include <memory>
#include <vector>
#include <queue>
#include <bitset>

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

void writeBitset(ofstream& out, const bitset<8>& bits, int validBits) {
    unsigned char byte = static_cast<unsigned char>(bits.to_ulong());
    out.write(reinterpret_cast<char*>(&byte), 1);
}

int main() {
    wstring file_name;
    wcout << L"Enter the Filename: ";
    wcin >> file_name;
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
        shared_ptr<HuffmanNode> left = pq.top(); pq.pop();
        shared_ptr<HuffmanNode> right = pq.top(); pq.pop();

        int sum = left->frequency + right->frequency;
        shared_ptr<HuffmanNode> newNode = make_shared<HuffmanNode>(L'\0', sum);
        newNode->left = left;
        newNode->right = right;

        pq.push(newNode);
    }

    shared_ptr<HuffmanNode> root = pq.top();
    unordered_map<wchar_t, wstring> huffmanCode;
    generateCodes(root, L"", huffmanCode);

    ofstream outputFile("output.bin", ios::binary);

    if (!outputFile.is_open()) {
        wcerr << L"Error opening output file: output.bin" << endl;
        return 1;
    }

    // Write the size of the frequency table
    int tableSize = frequencyTable.size();
    outputFile.write(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));

    // Write each character and its corresponding Huffman code
    for (const auto& pair : huffmanCode) {
        wchar_t ch = pair.first;
        wstring code = pair.second;
        int codeLength = code.length();
        outputFile.write(reinterpret_cast<char*>(&ch), sizeof(ch));
        outputFile.write(reinterpret_cast<char*>(&codeLength), sizeof(codeLength));
        string narrowCode = converter.to_bytes(code);
        outputFile.write(narrowCode.c_str(), codeLength);
    }

    // Write the Huffman encoded data
    bitset<8> bits;
    int bitIndex = 0;

    for (wchar_t ch : wFileContent) {
        wstring code = huffmanCode[ch];
        for (wchar_t bit : code) {
            bits[bitIndex++] = (bit == L'1');
            if (bitIndex == 8) {
                writeBitset(outputFile, bits, 8);
                bits.reset();
                bitIndex = 0;
            }
        }
    }

    // Write any remaining bits (pad the rest with zeros)
    if (bitIndex > 0) {
        writeBitset(outputFile, bits, bitIndex);
    }

    outputFile.close();
    wcout << L"Compressed data written to output.bin" << endl;

    return 0;
}
