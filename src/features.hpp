#ifndef WHATMUD_FEATURES_HPP
#define WHATMUD_FEATURES_HPP

namespace whatmud {

struct Features {
  // Client can understand UTF-8
  bool utf8 : 1 = true;
};

} // namespace whatmud

#endif
