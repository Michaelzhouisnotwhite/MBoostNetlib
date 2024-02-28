//
// Created by tssh on 24-2-22.
//

#include "HttpUtils.h"

#include <iomanip>
#include <memory>

#include "fmt/xchar.h"

namespace mhttplib {
void HttpHeader::SetContentType(const String& content_type) {
  header_["Content-Type"] = content_type;
}
void HttpHeader::SetContentLength(u64 length) {
  header_["Content-Length"] = std::to_string(length);
  chunked_ = false;
}
bool HttpHeader::chunked() const {
  return chunked_;
}
void HttpHeader::SetChunked(bool chunked) {
  if (chunked) {
    header_.erase("Content-Length");
  }
}
HttpHeader::HeaderType HttpHeader::GetHeader() const {
  return header_;
}
void HttpHeader::SetHeader(const String& key, const String& value) {
  operator[](key) = value;
}
String& HttpHeader::operator[](const String& key) {
  if (key == "Transfer-Encoding") {
    chunked_ = true;
    auto iter = header_.find("Content-Length");
    header_.erase(iter);
  }
  return header_[key];
}
auto HttpHeader::begin() -> HeaderType::iterator {
  return header_.begin();
}
auto HttpHeader::end() -> HeaderType::iterator {
  return header_.end();
}
bool HttpHeader::HasContentLength() const {
  auto iter = header_.find("Content-Length");
  if (auto chunk_iter = header_.find("Transfer-Encoding"); chunk_iter != header_.end()) {
    return false;
  }
  return iter != header_.end();
}
u64 HttpHeader::ContentLength() const {
  if (HasContentLength()) {
    return std::stoull(header_.at("Content-Length"));
  }
  return 0;
}
String HttpHeader::ToString() {
  String res;
  res.reserve(1500 * header_.size());
  for (const auto& [key, val] : header_) {
    res.append(fmt::format("{}: {}\r\n", key, val));
  }
  res.append("\r\n");
  return res;
}
bool HttpHeader::Empty() {
  return header_.empty();
}
u64 HttpRequest::RemainContentSize() const {
  return header.ContentLength() - body.length();
}
String HttpResponseBase::StatusText() const {
  if (StatusCode() < 300 || StatusCode() >= 200) {
    return "OK";
  }
  return "NOT OK";
}
int HttpResponseBase::StatusCode() const {
  return status_code_;
}
void HttpResponseBase::StatusCode(int code) {
  status_code_ = code;
}
String HttpResponseBase::HttpVersionString() const {
  return "HTTP/1.1";
}
void HttpResponseBase::SetHttpVersionString(const String& version) {
  version_ = version;
}
String HttpResponseBase::GetGMTTimeString() {
  std::time_t const now = std::time(nullptr);
  std::stringstream out;
  out << std::put_time(std::gmtime(&now), "%a, %d %b %Y %H:%M:%S");
  return out.str() + " GMT";
}

void HttpBaseResponse::Prepare() {
  ptr_ = 0;
  all_bytes_.clear();
  all_bytes_.reserve(1000);
  header["Server"] = "mboosthttp-server/dev";
  header["Date"] = GetGMTTimeString();
  header.SetContentLength(body.length());
  auto header_str = header.ToString();
  auto status_line = fmt::format("{} {} {}\r\n", HttpVersionString(), StatusCode(), StatusText());
  all_bytes_.insert(all_bytes_.end(), status_line.begin(), status_line.end());
  all_bytes_.insert(all_bytes_.end(), header_str.begin(), header_str.end());
  all_bytes_.insert(all_bytes_.end(), body.begin(), body.end());
}
Vec<char> HttpBaseResponse::Read(u64 size) {
  if (size + ptr_ >= all_bytes_.size()) {
    size = all_bytes_.size() - ptr_;
  }
  i64 offset;
  if (ptr_ >= std::numeric_limits<i64>::max()) {
    offset = std::numeric_limits<i64>::max();
  } else {
    offset = static_cast<i64>(ptr_);
  }
  Vec<char> res(all_bytes_.begin() + offset, all_bytes_.begin() + offset + size);
  if (ptr_ + size >= all_bytes_.size()) {
    ptr_ = all_bytes_.size();
  } else {
    ptr_ += size;
  }
  return res;
}
HttpHeader HttpBaseResponse::Header() const {
  return header;
}
u64 HttpRequestParser::Put(const Vec<char>& buffer, int& ec) {
  if (!req_) {
    req_ = std::make_shared<HttpRequest>();
  }
  u64 consume_size = 0;
  for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
    auto res = ec = MoveNextState(*iter);

    if (res == header_good) {
      for (const auto& [name, value] : header_lines_) {
        req_->header[name] = String(value.c_str());
      }
      if (!req_->header.HasContentLength() ||
          req_->header.ContentLength() == 0 && !req_->header.chunked()) {
        done_ = true;
        res = content_good;
      }
    }
    consume_size += 1;
    if (res == bad || res == content_good) {
      return consume_size;
    }
  }
  return consume_size;
}
bool HttpRequestParser::HeaderDone() const {
  return header_done_;
}
bool HttpRequestParser::Done() const {
  return done_;
}
void HttpRequestParser::Reset() {
  req_.reset();
  state_ = state::method_start;
  header_done_ = done_ = false;
  header_lines_.clear();
}
std::shared_ptr<HttpRequest> HttpRequestParser::Result() const {
  return req_;
}
int HttpRequestParser::ParseBody(char input) {
  if (req_->header.HasContentLength()) {
    if (req_->RemainContentSize()) {
      req_->body.push_back(input);
    }
    return !req_->RemainContentSize() ? content_good : indeterminate;
  }
  return bad;
}

int HttpRequestParser::MoveNextState(char input) {
  switch (state_) {
    case state::method_start: {
      if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      state_ = state::method;
      req_->method.push_back(input);
      return indeterminate;
    }
    case state::method: {
      if (input == ' ') {
        state_ = state::uri;
        return indeterminate;
      }
      if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      if (auto success = req_->method.push_back(input); !success) {
        return bad;
      }
      return indeterminate;
    }
    case state::uri: {
      if (input == ' ') {
        state_ = state::http_version_h;
        return indeterminate;
      }
      if (is_ctl(input)) {
        return bad;
      }
      if (auto success = req_->uri.push_back(input); !success) {
        return bad;
      }
      return indeterminate;
    }
    case state::http_version_h: {
      if (input == 'H') {
        state_ = state::http_version_t_1;
        return indeterminate;
      }
      return bad;
    }
    case state::http_version_t_1: {
      if (input == 'T') {
        state_ = state::http_version_t_2;
        return indeterminate;
      }
      return bad;
    }
    case state::http_version_t_2:
      if (input == 'T') {
        state_ = state::http_version_p;
        return indeterminate;
      } else {
        return bad;
      }
    case state::http_version_p:
      if (input == 'P') {
        state_ = state::http_version_slash;
        return indeterminate;
      } else {
        return bad;
      }
    case state::http_version_slash:
      if (input == '/') {
        req_->http_version_major = 0;
        req_->http_version_minor = 0;
        state_ = state::http_version_major_start;
        return indeterminate;
      } else {
        return bad;
      }
    case state::http_version_major_start:
      if (is_digit(input)) {
        req_->http_version_major = req_->http_version_major * 10 + input - '0';
        state_ = state::http_version_major;
        return indeterminate;
      } else {
        return bad;
      }
    case state::http_version_major:
      if (input == '.') {
        state_ = state::http_version_minor_start;
        return indeterminate;
      } else if (is_digit(input)) {
        req_->http_version_major = req_->http_version_major * 10 + input - '0';
        return indeterminate;
      } else {
        return bad;
      }
    case state::http_version_minor_start:
      if (is_digit(input)) {
        req_->http_version_minor = req_->http_version_minor * 10 + input - '0';
        state_ = state::http_version_minor;
        return indeterminate;
      } else {
        return bad;
      }
    case state::http_version_minor:
      if (input == '\r') {
        state_ = state::expecting_newline_1;
        return indeterminate;
      } else if (is_digit(input)) {
        req_->http_version_minor = req_->http_version_minor * 10 + input - '0';
        return indeterminate;
      } else {
        return bad;
      }
    case state::expecting_newline_1:
      if (input == '\n') {
        state_ = state::header_line_start;
        return indeterminate;
      } else {
        return bad;
      }
    case state::header_line_start:
      if (input == '\r') {
        state_ = state::expecting_newline_3;
        return indeterminate;
      } else if (!req_->header.Empty() && (input == ' ' || input == '\t')) {
        state_ = state::header_lws;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        header_lines_.emplace_back();
        if (auto scs = header_lines_.back().name.push_back(input); !scs) {
          return bad;
        }
        state_ = state::header_name;
        return indeterminate;
      }
    case state::header_lws: {
      if (input == '\r') {
        state_ = state::expecting_newline_2;
        return indeterminate;
      }
      if (input == ' ' || input == '\t') {
        return indeterminate;
      }
      if (is_ctl(input)) {
        return bad;
      }
      state_ = state::header_value;
      if (auto scs = header_lines_.back().value.push_back(input); !scs) {
        return bad;
      }
      return indeterminate;
    }
    case state::header_name: {
      if (input == ':') {
        state_ = state::space_before_header_value;
        return indeterminate;
      }
      if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      }
      header_lines_.back().name.push_back(input);
      return indeterminate;
    }
    case state::space_before_header_value: {
      if (input == ' ') {
        state_ = state::header_value;
        return indeterminate;
      }
      return bad;
    }
    case state::header_value: {
      if (input == '\r') {
        state_ = state::expecting_newline_2;
        return indeterminate;
      }
      if (is_ctl(input)) {
        return bad;
      }
      if (auto scs = header_lines_.back().value.push_back(input); !scs) {
        return bad;
      }
      return indeterminate;
    }
    case state::expecting_newline_2: {
      if (input == '\n') {
        state_ = state::header_line_start;
        return indeterminate;
      }
      return bad;
    }
    case state::expecting_newline_3:
      state_ = state::expecting_content;
      return (input == '\n') ? header_done_ = true, header_good : bad;
    case state::expecting_content: {
      if (!done_) {
        if (req_->header.HasContentLength()) {
          req_->body.push_back(input);
          if (!req_->RemainContentSize()) {
            done_ = true;
            return content_good;
          }
          return indeterminate;
        }
      }
      return bad;
    }
      return indeterminate;
    default:
      return bad;
  }
}
bool HttpRequestParser::is_char(int c) {
  return c >= 0 && c <= 127;
}
bool HttpRequestParser::is_ctl(int c) {
  return (c >= 0 && c <= 31) || (c == 127);
}
bool HttpRequestParser::is_tspecial(int c) {
  switch (c) {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case ' ':
    case '\t':
      return true;
    default:
      return false;
  }
}
bool HttpRequestParser::is_digit(int c) {
  return c >= '0' && c <= '9';
}

}  // namespace mhttplib