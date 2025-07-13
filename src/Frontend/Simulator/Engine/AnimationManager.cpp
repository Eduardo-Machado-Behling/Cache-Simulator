#include "Frontend/Simulator/Engine/AnimationManager.hpp"

AnimationManager::Animation::Animation(
    std::function<void(float, bool)> callback, float duration)
    : callback(callback), duration(duration), total(0) {}

bool AnimationManager::Animation::operator()(float elapsed_time) {
  total += elapsed_time;

  float progress = total / duration;
  if (progress > 1.0)
    progress = 1.0;

  callback(progress, reverse);
  return progress == 1.0;
}

void AnimationManager::Animation::reset() { total = 0; }

auto AnimationManager::operator()(float elapsed_time) -> void {
  for (auto it = tracks.begin(); it != tracks.end(); it++) {
    auto &track = *it;
    if (track.active)
      if (track(elapsed_time * playback)) {
        toRemove.push_back(it);
      }
  }

  for (auto rit = toRemove.rbegin(); rit != toRemove.rend(); rit++) {
	  tracks.erase(*rit);
	  toRemove.erase(rit.base());
  }
}

AnimationManager::AnimationTrack::AnimationTrack(
    std::vector<Animation> &animations)
    : i(0), animations(std::move(animations)) {}
AnimationManager::AnimationTrack::AnimationTrack(
    std::vector<Animation> &&animations)
    : i(0), animations(animations) {}

bool AnimationManager::AnimationTrack::operator()(float elapsed_time) {
  if (i >= animations.size())
    return true;

  animations[i].reverse = inverse;

  bool res = animations[i](elapsed_time);

  if (res && inverse) {
    inverse--;
    i = (i == 0) ? i : i - 1;
  } else if (res && autoNext) {
    animations[i++].reset();
  }

  return false;
}

void AnimationManager::AnimationTrack::reset() {
  i = 0;
  animations[0].reset();
}
size_t AnimationManager::AnimationTrack::size() { return animations.size(); }
void AnimationManager::AnimationTrack::prev() {
  inverse = 1;
  animations[i].reset();
}

void AnimationManager::AnimationTrack::next() {
  i = (i + 1) % animations.size();
  animations[i].reset();
}

auto AnimationManager::add_track(AnimationTrack &track) -> void {
  tracks.push_back(track);
}

auto AnimationManager::add_track(AnimationTrack track) -> void {
  tracks.push_back(track);
}

void AnimationManager::setPlaybackRate(float playbackRate) {
  playback = playbackRate;
}
