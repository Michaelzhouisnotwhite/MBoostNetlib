//
// Created by tssh on 24-2-22.
//

#ifndef HTTPUTILS_H
#define HTTPUTILS_H
#include <toy/type.h>

#include <boost/asio.hpp>
#include <optional>

#include "buffers.h"
namespace mhttplib {
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
  String& operator[](const String& key) {
    return header_[key];
  }
  bool HasContentLength() const;
  u64 ContentLength() const;
  String ToString();
  bool Empty() {
    return header_.empty();
  }

private:
  HeaderType header_;
  bool chunked_ = false;
};

class HttpRequest {
public:
  String body;
  MaxLenString method;
  MaxLenString uri;
  int http_version_major;
  int http_version_minor;
  HttpHeader header;
  u64 RemainContentSize() const;
};

class HttpResponseUtils {
public:
  virtual ~HttpResponseUtils() = default;

protected:
  virtual void Prepare() = 0;
  virtual Vec<char> Read(u64 size) = 0;
  virtual HttpHeader Header() const = 0;
};
class HttpResponseBase {
public:
  virtual ~HttpResponseBase() = default;

  virtual int StatusCode() const;
  virtual void StatusCode(int code);
  virtual String HttpVersionString() const;
  virtual void SetHttpVersionString(const String& version);

private:
  int status_code_ = 200;
  String version_;
};
class HttpBaseResponse : public HttpResponseBase, public HttpResponseUtils {
public:
  HttpHeader header;
  String body;
  void Prepare() override;
  Vec<char> Read(u64 size) override;
  HttpHeader Header() const override;

private:
  Vec<char> all_bytes_;
  u64 ptr_ = 0;
};
class HttpRequestParser {
public:
  enum ErrorCode : int { Success, ParseError, NeedMore };
  HttpRequestParser() = default;
  u64 Put(const Vec<char>& buffer, int& ec);
  bool HeaderDone() const;
  bool Done() const;
  void Reset();

private:
  int ParseBody(char input);
  struct HeaderLine {
    MaxLenString name{};
    MaxLenString value{};
  };
  VecDeque<HeaderLine> header_lines_;
  int MoveNextState(char input);
  enum result_type { header_good, bad, indeterminate, content_good };
  enum state {
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
  } state_ = method_start;
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
