//
// Created by tssh on 24-2-22.
//

#include "HttpUtils.h"

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
String HttpHeader::ToString() {
  String res;
  for (const auto& [key, val] : header_) {
    res.append(fmt::format("{}:{}\r\n", key, val));
  }
  res.append("\r\n");
  return res;
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

void HttpBaseResponse::Prepare() {
  ptr_ = 0;
  all_bytes_.clear();
  header.SetContentLength(body.length());
  auto header_str = header.ToString();
  auto status_line = fmt::format("{} {} ok\r\n", HttpVersionString(), StatusCode());
  all_bytes_.insert(all_bytes_.end(), status_line.begin(), status_line.end());
  all_bytes_.insert(all_bytes_.end(), header_str.begin(), header_str.end());
  all_bytes_.insert(all_bytes_.end(), body.begin(), body.end());
}
Vec<char> HttpBaseResponse::Read(u64 size) {
  if (size + ptr_ >= all_bytes_.size()) {
    size = all_bytes_.size() - ptr_;
  }
  i64 offset;
  if (ptr_ >= std::numeric_limits<i64>::max()){
    offset = std::numeric_limits<i64>::max();
  }else {
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
  for (auto iter = buffer.begin(); iter != buffer.end(); ++iter) {
    if (auto res = ec = MoveNextState(*iter); res == good || res == bad) {
      return iter - buffer.begin();
    }
  }
  ec = indeterminate;
  return buffer.size();
}
int HttpRequestParser::MoveNextState(char input) {
  switch (state_) {
    case method_start:
      if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        state_ = method;
        req_->method.push_back(input);
        return indeterminate;
      }
    case method:
      if (input == ' ') {
        state_ = uri;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        req_->method.push_back(input);
        return indeterminate;
      }
    case uri:
      if (input == ' ') {
        state_ = http_version_h;
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        req_->uri.push_back(input);
        return indeterminate;
      }
    case http_version_h:
      if (input == 'H') {
        state_ = http_version_t_1;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_t_1:
      if (input == 'T') {
        state_ = http_version_t_2;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_t_2:
      if (input == 'T') {
        state_ = http_version_p;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_p:
      if (input == 'P') {
        state_ = http_version_slash;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_slash:
      if (input == '/') {
        req_->http_version_major = 0;
        req_->http_version_minor = 0;
        state_ = http_version_major_start;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_major_start:
      if (is_digit(input)) {
        req_->http_version_major = req_->http_version_major * 10 + input - '0';
        state_ = http_version_major;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_major:
      if (input == '.') {
        state_ = http_version_minor_start;
        return indeterminate;
      } else if (is_digit(input)) {
        req_->http_version_major = req_->http_version_major * 10 + input - '0';
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_minor_start:
      if (is_digit(input)) {
        req_->http_version_minor = req_->http_version_minor * 10 + input - '0';
        state_ = http_version_minor;
        return indeterminate;
      } else {
        return bad;
      }
    case http_version_minor:
      if (input == '\r') {
        state_ = expecting_newline_1;
        return indeterminate;
      } else if (is_digit(input)) {
        req_->http_version_minor = req_->http_version_minor * 10 + input - '0';
        return indeterminate;
      } else {
        return bad;
      }
    case expecting_newline_1:
      if (input == '\n') {
        state_ = header_line_start;
        return indeterminate;
      } else {
        return bad;
      }
    case header_line_start:
      if (input == '\r') {
        state_ = expecting_newline_3;
        return indeterminate;
      } else if (!req_->header.Empty() && (input == ' ' || input == '\t')) {
        state_ = header_lws;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        header_lines_.emplace_back();
        header_lines_.back().name.push_back(input);
        state_ = header_name;
        return indeterminate;
      }
    case header_lws:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return indeterminate;
      } else if (input == ' ' || input == '\t') {
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        state_ = header_value;
        header_lines_.back().value.push_back(input);
        return indeterminate;
      }
    case header_name:
      if (input == ':') {
        state_ = space_before_header_value;
        return indeterminate;
      } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
        return bad;
      } else {
        header_lines_.back().name.push_back(input);
        return indeterminate;
      }
    case space_before_header_value:
      if (input == ' ') {
        state_ = header_value;
        return indeterminate;
      } else {
        return bad;
      }
    case header_value:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return indeterminate;
      } else if (is_ctl(input)) {
        return bad;
      } else {
        header_lines_.back().value.push_back(input);
        return indeterminate;
      }
    case expecting_newline_2:
      if (input == '\n') {
        state_ = header_line_start;
        return indeterminate;
      } else {
        return bad;
      }
    case expecting_newline_3:
      return (input == '\n') ? header_done_ = true, good : bad;
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