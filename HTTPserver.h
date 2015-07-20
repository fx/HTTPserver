// HTTPserver class
class HTTPserver : public Print
  {

  static const size_t MAX_KEY_LENGTH = 40;  // maximum size for a key
  char keyBuffer [MAX_KEY_LENGTH + 1];      // store here
  size_t keyBufferPos;                      // how much data we have collected

  static const size_t MAX_VALUE_LENGTH = 100;  // maximum size for a value
  char valueBuffer [MAX_VALUE_LENGTH + 1];     // store here
  size_t valueBufferPos;                       // how much data we have collected

  // state machine: possible states
  enum StateType {
    SKIP_INITIAL_LINES, // skip blank lines before the GET line
    GET_LINE,           // eg. GET /myblog/nick.html?foo=42 HTTP/1.1 (might be POST)
    SKIP_GET_SPACES_1,  // spaces after GET
    GET_PATHNAME,       // eg. /foo/myblog.html
    GET_ARGUMENT_NAME,  // eg. foo
    GET_ARGUMENT_VALUE, // eg. 42
    SKIP_GET_SPACES_2,  // spaces after arguments
    GET_HTTP_VERSION,   // eg. HTTP/1.1
    SKIP_TO_END_OF_LINE,
    START_LINE,         // start of another header line
    HEADER_NAME,        // eg. Accept:
    SKIP_HEADER_SPACES, // spaces after header name
    HEADER_VALUE,       // eg. text/html
    SKIP_COOKIE_SPACES, // spaces before cookie name
    COOKIE_NAME,        // eg. theme
    COOKIE_VALUE,       // eg. light
    POST_NAME,          // eg. action
    POST_VALUE,         // eg. add
  };
  // current state
  StateType state;

  // percent-encoding: possible states
  enum EncodePhaseType {
    ENCODE_NONE,
    ENCODE_GOT_PERCENT,
    ENCODE_GOT_FIRST_CHAR,
  };
  // percent-encoding: current state
  EncodePhaseType encodePhase;
  byte encodeByte;  // encoded byte being assembled (first nybble)

  // extra information about the key/value passed to a callback function
  // (bitmask - more than one might be set)
  enum  {
    FLAG_NONE                   = 0x00,    // no problems
    FLAG_KEY_BUFFER_OVERFLOW    = 0x01,    // the key was truncated
    FLAG_VALUE_BUFFER_OVERFLOW  = 0x02,    // the value was truncated
    FLAG_ENCODING_ERROR         = 0x04,    // %xx encoding error
  };
  byte flags;  // see above enum

  bool postRequest; // true if a POST type
  unsigned long contentLength;   // how long the POST data is
  unsigned long receivedLength;  // how much POST data we currently have
  Print * output;  // where to write output to

  // private methods (just used internally)

  // state change
  void newState (StateType what);
  // buffer handlers
  void addToKeyBuffer (const byte inByte);
  void addToValueBuffer (byte inByte);
  void clearBuffers ();
  // state handlers
  void handleNewline ();
  void handleSpace ();
  void handleText (const byte inByte);

  public:

    // constructor
    HTTPserver () { begin (NULL); }

    // re-initialize states
    void begin (Print * output_);

    // handle one incoming byte from the client
    void processIncomingByte (const byte inByte);

    // user handlers - override to do something with them
    virtual void processPostType        (const char * key, const byte flags) { }
    virtual void processPathname        (const char * key, const byte flags) { }
    virtual void processHttpVersion     (const char * key, const byte flags) { }
    virtual void processGetArgument     (const char * key, const char * value, const byte flags) { }
    virtual void processHeaderArgument  (const char * key, const char * value, const byte flags) { }
    virtual void processCookie          (const char * key, const char * value, const byte flags) { }
    virtual void processPostArgument    (const char * key, const char * value, const byte flags) { }

    // for outputting back to client
  	size_t write(uint8_t c);
    void printHTML (const char * message);

    // set to stop further processing (eg. on error)
    bool done;

    using Print::write;
  };  // end of HTTPserver