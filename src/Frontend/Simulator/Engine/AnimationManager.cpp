#include "Frontend/Simulator/Engine/AnimationManager.hpp"
#include <algorithm>

AnimationManager::Animation::Animation(std::function<void(float)> callback, float duration) : callback(callback), duration(duration), total(0){
}

bool AnimationManager::Animation::operator()(float elapsed_time){
	total += elapsed_time;

	float progress = total / duration;
	if(progress > 1.0)
		progress = 1.0;

	callback(progress);
	return progress == 1.0;
}


auto AnimationManager::operator()(float elapsed_time) -> void {
	for (auto& track : tracks){
		if(track.empty())
			continue;

		if(track.front()(elapsed_time))
			track.pop_front();
	}

	std::remove_if(tracks.begin(), tracks.end(), [](auto& v){
				return v.empty();
			}
	);
}

auto AnimationManager::add_track(AnimationTrack& track) -> void {
	tracks.push_back(track);
}
auto AnimationManager::add_track(AnimationTrack track) -> void {
	tracks.push_back(track);
}
