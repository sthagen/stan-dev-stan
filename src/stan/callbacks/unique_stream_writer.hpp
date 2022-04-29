#ifndef STAN_CALLBACKS_UNIQUE_STREAM_WRITER_HPP
#define STAN_CALLBACKS_UNIQUE_STREAM_WRITER_HPP

#include <stan/callbacks/writer.hpp>
#include <ostream>
#include <vector>
#include <string>

namespace stan {
namespace callbacks {

/**
 * <code>unique_stream_writer</code> is an implementation
 * of <code>writer</code> that holds a unique pointer to the stream it is
 * writing to.
 * @tparam Stream A type with with a valid `operator<<(std::string)`
 */
template <typename Stream>
class unique_stream_writer final : public writer {
 public:
  /**
   * Constructs a unique stream writer with an output stream
   * and an optional prefix for comments.
   *
   * @param[in, out] A unique pointer to a type inheriting from `std::ostream`
   * @param[in] comment_prefix string to stream before each comment line.
   *  Default is "".
   */
  explicit unique_stream_writer(std::unique_ptr<Stream>&& output,
                                const std::string& comment_prefix = "")
      : output_(std::move(output)), comment_prefix_(comment_prefix) {}

  unique_stream_writer() = default;
  unique_stream_writer(unique_stream_writer& other) = delete;
  unique_stream_writer(unique_stream_writer&& other)
      : output_(std::move(other.output_)),
        comment_prefix_(std::move(other.comment_prefix_)) {}
  inline unique_stream_writer& operator=(unique_stream_writer<Stream>&& other) {
    this->output_ = std::move(other.output_);
    this->comment_prefix_ = other.comment_prefix_;
    return *this;
  }
  /**
   * Virtual destructor
   */
  virtual ~unique_stream_writer() {}

  /**
   * Writes a set of names on a single line in csv format followed
   * by a newline.
   *
   * Note: the names are not escaped.
   *
   * @param[in] names Names in a std::vector
   */
  void operator()(const std::vector<std::string>& names) {
    if (output_ == nullptr)
      return;
    write_vector(names);
  }
  /**
   * Get the underlying stream
   */
  inline auto& get_stream() noexcept { return *output_; }

  /**
   * Writes a set of values in csv format followed by a newline.
   *
   * Note: the precision of the output is determined by the settings
   *  of the stream on construction.
   *
   * @param[in] state Values in a std::vector
   */
  void operator()(const std::vector<double>& state) { write_vector(state); }

  void operator()(const std::tuple<Eigen::VectorXd, Eigen::VectorXd>& state) {
    write_vector(std::get<0>(state));
    write_vector(std::get<1>(state));
  }

  void operator()(const Eigen::MatrixXd& states) {
    if (output_ == nullptr)
      return;
    Eigen::IOFormat CommaInitFmt(Eigen::StreamPrecision, Eigen::DontAlignCols,
                                 ", ", "", "", "\n", "", "");
    *output_ << states.transpose().format(CommaInitFmt);
  }

  /**
   * Writes the comment_prefix to the stream followed by a newline.
   */
  void operator()() {
    if (output_ == nullptr)
      return;
    *output_ << comment_prefix_ << std::endl;
  }

  /**
   * Writes the comment_prefix then the message followed by a newline.
   *
   * @param[in] message A string
   */
  void operator()(const std::string& message) {
    if (output_ == nullptr)
      return;
    *output_ << comment_prefix_ << message << std::endl;
  }

 private:
  /**
   * Output stream
   */
  std::unique_ptr<Stream> output_{nullptr};

  /**
   * Comment prefix to use when printing comments: strings and blank lines
   */
  std::string comment_prefix_{"# "};

  /**
   * Writes a set of values in csv format followed by a newline.
   *
   * Note: the precision of the output is determined by the settings
   *  of the stream on construction.
   *
   * @param[in] v Values in a std::vector
   */
  template <class T>
  void write_vector(const std::vector<T>& v) {
    if (output_ == nullptr)
      return;
    if (v.empty()) {
      return;
    }
    auto last = v.end();
    --last;
    for (auto it = v.begin(); it != last; ++it) {
      *output_ << *it << ",";
    }
    *output_ << v.back() << std::endl;
  }
};

}  // namespace callbacks
}  // namespace stan

#endif
