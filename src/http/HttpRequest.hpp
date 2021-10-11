#ifndef MYMUDUO_SRC_HTTP_HTTPREQUEST_HPP_
#define MYMUDUO_SRC_HTTP_HTTPREQUEST_HPP_

#include "TimeUtil.hpp"

#include <map>
#include <cassert>

namespace mymuduo {

class HttpRequest {
 public:
  enum class Method {
    Invalid, Get, Post, Head, Put, Delete
  };
  enum class Version {
    Unknown, Http10, Http11
  };
  HttpRequest() : method_(Method::Invalid), version_(Version::Unknown) {}
  void setVersion(Version v) { version_ = v; }
  Version getVersion() const { return version_; }
  bool setMethod(const char *start, const char *end) {
    assert(method_ == Method::Invalid);
    std::string m(start, end);
    if (m == "GET") { method_ = Method::Get; }
    else if (m == "POST") { method_ = Method::Post; }
    else if (m == "HEAD") { method_ = Method::Head; }
    else if (m == "PUT") { method_ = Method::Put; }
    else if (m == "DELETE") { method_ = Method::Delete; }
    else { method_ = Method::Invalid; }
    return method_ != Method::Invalid;
  }
  Method method() const { return method_; }
  const char *methodString() const {
    switch (method_) {
      case Method::Get: return "GET";
      case Method::Post: return "POST";
      case Method::Head: return "HEAD";
      case Method::Put: return "PUT";
      case Method::Delete: return "DELETE";
      default: break;
    }
    return "UNKNOWN";
  }
  void setPath(const char *start, const char *end) {
    path_.assign(start, end);
  }
  const std::string &path() const { return path_; }
  void setQuery(const char *start, const char *end) {
    query_.assign(start, end);
  }
  const std::string &query() const {
    return query_;
  }
  void setReceiveTime(time_point t) {
    receiveTime_ = t;
  }
  time_point receiveTime() const { return receiveTime_; }
  void addHeader(const char *start, const char *colon, const char *end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) {
      ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && isspace(value[value.size() - 1])) {
      value.resize(value.size() - 1);
    }
    headers_[field] = value;
  }
  std::string getHeader(const std::string &field) const {
    std::string result;
    auto it = headers_.find(field);
    if (it != headers_.end()) {
      result = it->second;
    }
    return result;
  }
  const std::map<std::string, std::string> &headers() const { return headers_; }
  void swap(HttpRequest &that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    std::swap(receiveTime_, that.receiveTime_);
    headers_.swap(that.headers_);
  }
 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  time_point receiveTime_;
  std::map<std::string, std::string> headers_;
};

} // namespace mymuduo

#endif //MYMUDUO_SRC_HTTP_HTTPREQUEST_HPP_
