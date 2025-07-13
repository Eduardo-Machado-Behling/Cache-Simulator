#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include <concepts>
#include <functional>
#include <initializer_list>
#include <list>

template <typename T>
concept Lerpable = requires(T a, T b, float scalar) {
  { a + (b - a) * scalar } -> std::convertible_to<T>;
};

struct AnimationManager {
  struct Animation {
    Animation(std::function<void(float, bool)> callback, float duration);

    bool operator()(float elapsed_time);
    void reset();

    bool reverse = false;

  private:
    std::function<void(float, bool)> callback;
    float duration;
    float total;
    bool activate = true;
  };

  struct AnimationTrack {
    AnimationTrack(std::vector<Animation> &animations);
    AnimationTrack(std::vector<Animation> &&animations);

    bool operator()(float elapsed_time);

    void reset();
    size_t size();

    void prev();
    void next();

    size_t i = 0;
    bool autoNext = true;
    bool active = true;
    size_t inverse = 0;

  private:
    std::vector<Animation> animations;
  };

  void operator()(float elapsed_time);
  void add_track(AnimationTrack &track);
  void add_track(AnimationTrack track);
  void setPlaybackRate(float playbackRate);

  float getPlaybackRate() { return playback; }

  template <class... Args> AnimationTrack &emplace(Args &&...args) {
    tracks.emplace_back(std::forward<Args>(args)...);
    return tracks.back();
  }

  template <Lerpable T> static auto lerp(const T &a, const T &b, float t) -> T {
    return a + (b - a) * t;
  }

private:
  float playback = 1.0;
  std::list<AnimationTrack> tracks;
  std::vector<std::list<AnimationTrack>::iterator> toRemove;
};

#endif
