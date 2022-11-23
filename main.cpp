#include<SDL.h>
#include<cstdlib>
#include<cmath>
#include<vector>
#include<iostream>
using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
constexpr int width = 640, height = 480;
constexpr double pi{ 3.14159265358979323846 };
enum class colors { red, green, blue, yellow, purple, white, black };

void set_color(colors c) {
	switch (c) {
	case colors::black:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		break;
	case colors::white:
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		break;
	case colors::blue:
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		break;
	case colors::green:
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		break;
	case colors::red:
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			break;
	case colors::yellow:
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		break;
	case colors::purple:
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
			break;
	}
}
void set_color(colors c, double dist) {
	switch (c) {
	case colors::black:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		break;
	case colors::white:
		SDL_SetRenderDrawColor(renderer, 155/dist, 155/dist, 155/dist, 255);
		break;
	case colors::blue:
		SDL_SetRenderDrawColor(renderer, 0, 0, 155/dist, 255);
		break;
	case colors::green:
		SDL_SetRenderDrawColor(renderer, 0, 155/dist, 0, 255);
		break;
	case colors::red:
		SDL_SetRenderDrawColor(renderer, 155/dist, 0, 0, 255);
		break;
	case colors::yellow:
		SDL_SetRenderDrawColor(renderer, 155/dist, 155/dist, 0, 255);
		break;
	case colors::purple:
		SDL_SetRenderDrawColor(renderer, 155/dist, 0, 155/dist, 255);
		break;
	}
}

struct vector2 {
	double x, y;
	vector2()= default;
	vector2(double d1, double d2) :x(d1), y(d2) {

	}
	vector2 operator+(const vector2& v) const {
		return { x + v.x,y + v.y };
	}
	vector2 operator-() const {
		return {-x,-y};
	}
	vector2 operator-(const vector2& v) const {
		return *this + (-v);
	}
	vector2& operator+=(const vector2& v) {
		this->x += v.x;
		this->y += v.y;
		return *this;
	}
	vector2& operator=(std::initializer_list<double>&l) {
		auto it = l.begin();
		this->x = *(it);
		this->y = *(it + 1);
		return *this;
	}
	vector2 operator*(double d) const {
		return { d * x,d * y };
	}
	double magnitude() const {
		return sqrt(x*x + y*y);
	}
	vector2 unit() const {
		auto d = 1/(this->magnitude());
		return (*this) * d;
	}
	vector2 normal() const {
		return { -(this->y),this->x };
	}
};
inline vector2 operator*(double d, const vector2& v) {
	return v * d;
}
inline double cross(const vector2& v1, const vector2& v2) {
	return (v1.x) * (v2.y) - (v1.y) * (v2.x);
}
inline double dot(const vector2& v1, const vector2& v2) {
	return (v1.x) * (v2.x) + (v1.y) * (v2.y);
}
inline double distance_squared(const vector2& v1, const vector2& v2) {
	return (v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y);
}

struct input {
	bool w=false, a=false, s=false, d=false, left=false, right=false, quit=false;
};

struct player {
	const double speed = 0.05;
	const double turn_speed = 0.008;
	vector2 pos{50,50};
	double angle=0;
	constexpr static double fov=1.57; //the trig functions work in radians my god i was dumb *facepalm* well now i understand why i was seeing 80 fucking times the same room
	void forward() {
		pos += speed*vector2(cos(angle), sin(angle));
	}
	void backwards() {
		pos += -speed * vector2(cos(angle), sin(angle));
	}
	void strife_left() {
		pos += speed*vector2(cos(angle), sin(angle)).normal();
	}
	void strife_right() {
		pos += -speed * vector2(cos(angle), sin(angle)).normal();
	}
	void counterclockwise() {
		angle += turn_speed;
	}
	void clockwise() {
		angle -= turn_speed;
	}
};

struct wall {
	vector2 start, end, direction;
	colors wall_color=colors::red;
	wall(const vector2& s, const vector2& e) :start(s), end(e) {
		direction = end - start;
	}
	wall(const vector2& s, const vector2& e, colors c) :wall(s,e) {
		wall_color = c;
	}
	wall(double x1, double y1, double x2, double y2) : start{ x1, y1 }, end{ x2, y2 } {
		direction = end - start;
	}
	wall(double x1, double y1, double x2, double y2, colors c) : wall(x1,y1,x2,y2) {
		wall_color = c;
	}
};

struct scene {
	std::vector<wall> walls;
};

struct ray {
	constexpr static double coeff = -(player::fov) / (width - 1);
	vector2 pos;
	double angle;
	vector2 direction;
	ray(const player& p, int column) : pos(p.pos) {
		angle = coeff * column + p.angle + player::fov / 2;
		direction = { cos(angle),sin(angle) };
	}
	std::pair<double,colors> collision(const wall& w) {
		auto numerator1 = cross(pos - w.start, direction);
		auto numerator2 = cross(pos - w.start, w.direction);
		auto denominator = cross(w.direction, direction);
		if (denominator == 0) {
			return { DBL_MAX, colors::black };
		}
		if (auto u = numerator1 / denominator, t=numerator2/denominator; 0 <= u and u <= 1 and t>=0) {
			//distance between pos+t*direction and the origin (pos) is entirely on the second term t*direction,
			//here I return the square of the distance to avoid doing sqrt, so (t*direction).(t*direction)
			return {dot(t*direction,t*direction), w.wall_color};
		}
		else {
			return { DBL_MAX, colors::black };
		}
		//pos + t * direction = w.start + u * w.direction
		//(pos - w.start) x direction/(w.direction x direction) = u (denominator!=0 and 0<=u<=1 for collision)
		// t = (pos - w.start) x w.direction/(w.direction x direction) t>=0 is also necessary for collision
	}
};

void camera(const scene& map, const player& p) {
	for (int col = 0; col < width; ++col) {
		//create ray at the player's position with the angle according to the column
		ray light(p, col);
		double min_dist = DBL_MAX;
		colors color_to_draw = colors::black;
		for (auto& w : map.walls) {
			auto [dist,color]=light.collision(w);
			if (dist < min_dist) {
				min_dist = dist;
				color_to_draw = color;
			}
		}
		//for every wall, check for a possible collision with it, the closest collision wins
		//draw a 1 pixel wide rectangle with the color of the wall on the specific column
		min_dist = sqrt(min_dist);
		set_color(color_to_draw, min_dist/3);
		SDL_RenderDrawLine(renderer,col,height/2 - 2000/(min_dist*cos(light.angle-p.angle)),col,height/2 + 2000/ (min_dist * cos(light.angle - p.angle)));
	}
}

//create and destroy SDL content
int init(int width, int height) {
	SDL_Init(SDL_INIT_EVERYTHING);
	return SDL_CreateWindowAndRenderer(width,height,SDL_WINDOW_SHOWN,&window,&renderer);
}
void quit() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

//the holy trinity of the main game loop
void get_input(input& in) {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_KEYDOWN:
			if (e.key.keysym.sym == SDLK_w) in.w = true;
			if (e.key.keysym.sym == SDLK_a) in.a = true;
			if (e.key.keysym.sym == SDLK_s) in.s = true;
			if (e.key.keysym.sym == SDLK_d) in.d = true;
			if (e.key.keysym.sym == SDLK_LEFT) in.left = true;
			if (e.key.keysym.sym == SDLK_RIGHT) in.right = true;
			if( e.key.keysym.sym == SDLK_ESCAPE) in.quit = true;
			break;
		case SDL_KEYUP:
			if (e.key.keysym.sym == SDLK_w) in.w = false;
			if (e.key.keysym.sym == SDLK_a) in.a = false;
			if (e.key.keysym.sym == SDLK_s) in.s = false;
			if (e.key.keysym.sym == SDLK_d) in.d = false;
			if (e.key.keysym.sym == SDLK_LEFT) in.left = false;
			if (e.key.keysym.sym == SDLK_RIGHT) in.right = false;
			break;
		}
	}
}
void update(player& p, const input& in) {
	if(in.w) p.forward();
	if(in.a) p.strife_left();
	if(in.s) p.backwards();
	if(in.d) p.strife_right();
	if (in.left) p.counterclockwise();
	if (in.right) p.clockwise();
}
void render(const scene& map, const player& p) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	camera(map, p);
	SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
	std::atexit(quit);
	init(640, 480);
	player p;
	input in;
	scene first_level;

	wall top{ {0,0},{100,0}, colors::purple};
	wall left{ {0,0},{0,100}, colors::red };
	wall right{ {100,0},{100,100}, colors::green };
	wall bottom{ {0,100},{100,100},colors::yellow };

	first_level.walls.push_back(top);
	first_level.walls.push_back(left);
	first_level.walls.push_back(right);
	first_level.walls.push_back(bottom);

	while (true) {
		get_input(in);
		if(in.quit) {
			break;
		}
		update(p,in);
		render(first_level,p);
	}
	return 0;
}