//
// Created by tssh on 24-2-22.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H
#include <toy/type.h>

#include <boost/asio.hpp>
#include <optional>

#include "buffers.h"

namespace mnet {
struct MaxLenString : public String {
  explicit MaxLenString(u64 max_size = 8190) : String(), max_len_(max_size) {
  }
  bool push_back(const char& val) {
    if (String::length() >= max_len_) {
      return false;
    }
    String::push_back(val);
    return true;
  }

private:
  u64 max_len_;
};
class HttpHeader {
public:
  using HeaderType = std::map<String, String>;
  void SetContentType(const String& content_type);
  void SetContentLength(u64 length);
  String ContentType();
  [[nodiscard]] bool chunked() const;
  void SetChunked(bool chunked);
  HeaderType GetHeader() const;
  void SetHeader(const String& key, const String& value);
  String& operator[](const String& key);
  auto begin() -> HeaderType::iterator;
  auto end() -> HeaderType::iterator;
  bool HasContentLength() const;
  u64 ContentLength() const;
  String ToString();
  bool Empty();

private:
  HeaderType header_;
  bool chunked_ = false;
};

class HttpRequest {
public:
  String body;
  MaxLenString method;
  MaxLenString uri;

  boost::asio::ip::address addr;
  boost::asio::ip::port_type port;

  int http_version_major;
  int http_version_minor;
  HttpHeader header;
  u64 RemainContentSize() const;
};

class HttpResponseBase {
public:
  virtual void SetStatusCode(int code);
  virtual void SetHttpVersionString(const String& version);
  virtual String StatusText() const;
  virtual int StatusCode() const;
  virtual String HttpVersionString() const;
  virtual String GetGMTTimeString();

  virtual ~HttpResponseBase() = default;
  virtual void Prepare() = 0;
  virtual Vec<char> Read(u64 size) = 0;
  virtual HttpHeader Header() const = 0;

protected:
private:
  int status_code_ = 200;
  String version_;
};
class HttpBaseResponse : public HttpResponseBase,
                         public std::enable_shared_from_this<HttpBaseResponse> {
public:
  HttpHeader header;
  String body;
  void Prepare() override;
  Vec<char> Read(u64 size) override;
  HttpHeader Header() const override;
  std::shared_ptr<HttpBaseResponse> Body(const String& body);
  std::shared_ptr<HttpBaseResponse> Header(HttpHeader header);
  std::shared_ptr<HttpBaseResponse> StatusCode(int code);
  static std::shared_ptr<HttpBaseResponse> Create();

private:
  Vec<char> all_bytes_;
  u64 ptr_ = 0;
};
class HttpRequestParser {
public:
  enum ParserResult { header_good, bad, indeterminate, content_good };
  HttpRequestParser() = default;
  u64 Put(const Vec<char>& buffer, int& ec);
  bool HeaderDone() const;
  bool Done() const;
  void Reset();
  std::shared_ptr<HttpRequest> Result() const;

private:
  int ParseBody(char input);
  struct HeaderLine {
    MaxLenString name{};
    MaxLenString value{};
  };
  VecDeque<HeaderLine> header_lines_;
  int MoveNextState(char input);

  enum class state {
    method_start,
    method,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3,
    expecting_content
  } state_ = state::method_start;
  u64 content_counter_ = 0;
  bool header_done_ = false;
  bool done_ = false;
  u32 max_http_line_ = 8029;
  std::shared_ptr<HttpRequest> req_;
  bool is_char(int c);

  bool is_ctl(int c);

  bool is_tspecial(int c);

  bool is_digit(int c);
};
}  // namespace mhttplib

#endif  // HTTPUTILS_H
