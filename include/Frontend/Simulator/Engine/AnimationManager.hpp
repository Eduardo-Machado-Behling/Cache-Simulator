#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include <functional>
#include <list>
#include <concepts>

template<typename T>
concept Lerpable = requires(T a, T b, float scalar) {
	{a + (b-a)* scalar } -> std::convertible_to<T>;
};

struct AnimationManager {
	struct Animation {
		Animation(std::function<void(float)> callback, float duration);

		bool operator()(float elapsed_time);

	private:
		std::function<void(float)> callback;
		float duration;
		float total;
	};

	using AnimationTrack = std::list<Animation>;

	void operator()(float elapsed_time);
	void add_track(AnimationTrack& track);
	void add_track(AnimationTrack track);

	template <Lerpable T>
	static auto lerp(const T& a, const T& b, float t) -> T{
		return a + (b-a)*t;
	}

private:
	std::vector<AnimationTrack> tracks;
};

#endif
