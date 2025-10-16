#ifndef STREAM_H_
#define STREAM_H_
#define MAX_SIZE 512

#include <string>
#include <vector>
#include <istream>
#include <memory>

class InputStream; // forward declaration

// Manager for parsed input segments (nodes), utilizing InputStream
class Stream {
public:
	Stream() = default;
    ~Stream() = default;

    // Parsing entry (fills internal nodes from input)
    friend std::istream& operator>>(std::istream& in, Stream& stream);

    // Pipeline access
    bool empty() const;
    InputStream* current();
    const InputStream* current() const;
    InputStream* next();
    const InputStream* next() const;
    bool advance();
    void clear();

    // Internal helpers reused by operator>>
    void split(const std::string& line);

    // Append a parsed node
    void insert(InputStream* node);

private:
    std::vector<std::unique_ptr<InputStream>> nodes;
    int pos = 0;

    std::vector<std::string> pipeline; // temporary storage for pipeline segments
};

// Single parsed segment (command [option] [argument [argument2 [argument3]]])
class InputStream {
public:
    explicit InputStream(const std::string& line); // parse and populate fields
    virtual ~InputStream() = default;

    const std::string& getCommand()  const;
    const std::string& getOption()   const;
    const std::string& getArgument() const;
    const std::string& getArgument2() const;
    const std::string& getArgument3() const;


    void setArgument(const std::string& a);
    void setArgument2(const std::string& a);
    void setArgument3(const std::string& a);
    void setCommand(const std::string& c);
    void setOption(const std::string& o);

    // Redirection accessors
    const std::string& getInRedirect() const;
    const std::string& getOutRedirect() const;
    bool isAppendOut() const;

    void setInRedirect(const std::string& in);
    void setOutRedirect(const std::string& out, bool append);

    // Track whether the user provided an explicit argument token (first argument only)
    bool hasExplicitArgument() const;
    void setHasExplicitArgument(bool v);

protected:
    InputStream() = default; // for derived classes

    void appendArgumentLine(const std::string& line); // used by multiline and file reading

    void parse(const std::string& line);              // tokenize input line
    std::string nextToken(const std::string& line, int& i); // helper

    std::string command;
    std::string option;
    std::string argument;   // first argument or input text
    std::string argument2;  // second argument (e.g., what)
    std::string argument3;  // third argument (e.g., with)

private:
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


// Stream
inline bool Stream::empty() const { return pos >= nodes.size(); }
inline const InputStream* Stream::current() const { return empty() ? nullptr : nodes[pos].get(); }
inline InputStream* Stream::current() { return const_cast<InputStream*>(static_cast<const Stream&>(*this).current()); }
inline const InputStream* Stream::next() const { return (pos + 1 < nodes.size()) ? nodes[pos + 1].get() : nullptr; }
inline InputStream* Stream::next() { return const_cast<InputStream*>(static_cast<const Stream&>(*this).next()); }
inline bool Stream::advance() { if (empty()) return false; ++pos; return !empty(); }
inline void Stream::insert(InputStream* node) { nodes.emplace_back(node); }


inline const std::string& InputStream::getCommand() const { return command; }
inline const std::string& InputStream::getOption() const { return option; }
inline const std::string& InputStream::getArgument() const { return argument; }
inline const std::string& InputStream::getArgument2() const { return argument2; }
inline const std::string& InputStream::getArgument3() const { return argument3; }

inline void InputStream::setArgument(const std::string& a) { argument = a; }
inline void InputStream::setArgument2(const std::string& a) { argument2 = a; }
inline void InputStream::setArgument3(const std::string& a) { argument3 = a; }
inline void InputStream::setCommand(const std::string& c) { command = c; }
inline void InputStream::setOption(const std::string& o) { option = o; }

inline const std::string& InputStream::getInRedirect() const { return inRedirect; }
inline const std::string& InputStream::getOutRedirect() const { return outRedirect; }
inline bool InputStream::isAppendOut() const { return appendOut; }

inline void InputStream::setInRedirect(const std::string& in) { inRedirect = in; }
inline void InputStream::setOutRedirect(const std::string& out, bool append) { outRedirect = out; appendOut = append; }

inline bool InputStream::hasExplicitArgument() const { return explicitArgument; }
inline void InputStream::setHasExplicitArgument(bool v) { explicitArgument = v; }

#endif // STREAM_H_
