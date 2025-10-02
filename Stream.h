#ifndef STREAM_H_
#define STREAM_H_
#define MAX_SIZE 512

#include <string>
#include <vector>
#include <istream>

class InputStream; // forward declaration

// Manager for parsed input segments (singleton)
class Stream {
public:
    static Stream* instance();

    void insert(InputStream* node);          // append node to list
    void split(const std::string& line);     // split raw line into pipeline segments

    void setFirst(InputStream* first);
    InputStream* getFirst() const;

    void clear();                            // delete all nodes & reset

    friend std::istream& operator>>(std::istream& in, Stream& stream);

private:
    Stream();
    ~Stream();
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    std::vector<std::string> pipeline;       // temporary storage for pipeline segments
    InputStream* first;                      // head of linked list
};

// Single parsed segment (command [option] [argument [argument2 [argument3]]])
class InputStream {
public:
    explicit InputStream(const std::string& line); // parse and populate fields
    virtual ~InputStream() = default;

    InputStream* getNext() const { return next; }
    void setNext(InputStream* n) { next = n; }

    const std::string& getCommand()  const { return command; }
    const std::string& getOption()   const { return option; }
    const std::string& getArgument() const { return argument; }
    const std::string& getArgument2() const { return argument2; }
    const std::string& getArgument3() const { return argument3; }

    void setArgument(const std::string& a) { argument = a; }
    void setArgument2(const std::string& a) { argument2 = a; }
    void setArgument3(const std::string& a) { argument3 = a; }
    void setCommand(const std::string& c) { command = c; }
    void setOption(const std::string& o) { option = o; }

    // Redirection accessors
    const std::string& getInRedirect() const { return inRedirect; }
    const std::string& getOutRedirect() const { return outRedirect; }
    bool isAppendOut() const { return appendOut; }

    void setInRedirect(const std::string& in) { inRedirect = in; }
    void setOutRedirect(const std::string& out, bool append) { outRedirect = out; appendOut = append; }

    // Track whether the user provided an explicit argument token (first argument only)
    bool hasExplicitArgument() const { return explicitArgument; }
    void setHasExplicitArgument(bool v) { explicitArgument = v; }

protected:
    InputStream() = default; // for derived classes

    void appendArgumentLine(const std::string& line); // used by multiline and file reading

    void parse(const std::string& line);              // tokenize input line
    std::string nextToken(const std::string& line, size_t& i); // helper

    std::string command;
    std::string option;
    std::string argument;   // first argument or input text
    std::string argument2;  // second argument (e.g., what)
    std::string argument3;  // third argument (e.g., with)

private:
    InputStream* next = nullptr;
    std::string inRedirect;
    std::string outRedirect;
    bool appendOut = false;
    bool explicitArgument = false;
};

// Specialized segment that loads argument from a file
class FileStream : public InputStream {
public:
    FileStream(const std::string& command,
               const std::string& option,
               const std::string& filePath);

    void readFromFile(const std::string& filePath);
};

#endif // STREAM_H_
