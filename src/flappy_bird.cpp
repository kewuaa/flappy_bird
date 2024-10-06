#include <cstdio>
#include <cassert>
#include <deque>
#include <format>
#include <cmath>
#include <random>

#include "raylib.h"

#include "config.hpp"


enum class GameStatus {
    PENDING,
    RUNNING,
    PAUSE,
    ENDING
} game_status = GameStatus::PENDING;
int score = 0;


class Pillar;


class Bird {
    public:
        constexpr static int SIZE = GAME_WIDTH / 10;
    private:
        constexpr static int ACTION_NUM = 4;
        constexpr static int IDLE_SPACE = GAME_FPS / 6;
        constexpr static int POS_X = SIZE;
        constexpr static double GRAVITY = 0.61;
        constexpr static double JUMP_SPEED = 9.8;
        Texture2D textures[ACTION_NUM];
        int index {0};
        double y {(GAME_HEIGHT - SIZE) / 2.};
        double v {0.};

        void update_index() noexcept {
            index += 1;
            if (index == ACTION_NUM) {
                index = 0;
            }
        }
    public:
        Bird() {
            for (int i = 0; i < ACTION_NUM; i++) {
                auto path = std::format("resource/bird-{}.png", i);
                auto img = LoadImage(path.c_str());
                ImageResize(&img, SIZE, SIZE);
                textures[i] = LoadTextureFromImage(img);
                UnloadImage(img);
            }
        }

        ~Bird() {
            for (int i = 0; i < ACTION_NUM; i++) {
                UnloadTexture(textures[i]);
            }
        }

        void update() noexcept {
            static char count = 0;
            count += 1;
            v += GRAVITY;
            if (y < GAME_HEIGHT + SIZE || v < 0) {
                y += v;
            }
            if (count % IDLE_SPACE == 0) {
                update_index();
            }
        }

        void draw() const noexcept {
            DrawTexture(textures[index], POS_X, (int)std::round(y), WHITE);
        }

        inline void jump() noexcept {
            v = -JUMP_SPEED;
        }

        void reset() noexcept {
            index = 0;
            y  = (GAME_HEIGHT - SIZE) / 2.;
            v  = 0.;
        }

        inline bool hit_bottom() const noexcept {
            return y + SIZE > GAME_HEIGHT;
        }

        inline bool hit_top() const noexcept {
            return y < 0;
        }

        friend bool collision_detection(const Bird& bird, const Pillar& pillar) noexcept;
};


class Pillar {
    private:
        constexpr static int WIDTH = GAME_WIDTH / 10;
        constexpr static int MIN_HEIGHT = GAME_HEIGHT / 4;
        constexpr static int MAX_HEIGHT = GAME_HEIGHT / 2;
        constexpr static double INIT_V = 1.;
        struct Pipe {
            double x;
            const double y;
            Pipe(double x, double y): x {x}, y {y} {}
        };
        Texture2D texture;
        std::deque<Pipe> pipe_list;
        double v {INIT_V};
        std::mt19937 engine;
        std::uniform_int_distribution<> dist;

        void new_pillar() {
            auto v = dist(engine) % 2;
            auto height = dist(engine);
            if (v) {
                pipe_list.emplace_back(GAME_WIDTH, height - MAX_HEIGHT);
            } else {
                pipe_list.emplace_back(GAME_WIDTH, GAME_HEIGHT - height);
            }
        }
    public:
        Pillar() {
            auto path = "resource/pillar.png";
            auto img = LoadImage(path);
            ImageResize(&img, WIDTH, MAX_HEIGHT);
            texture = LoadTextureFromImage(img);
            UnloadImage(img);

            std::random_device device;
            engine = std::mt19937(device());
            dist = std::uniform_int_distribution<>(MIN_HEIGHT, MAX_HEIGHT);

            new_pillar();
        }

        ~Pillar() {
            UnloadTexture(texture);
        }

        void update() noexcept {
            assert(!pipe_list.empty() && "at least one pipe is needed");
            for (auto& pos : pipe_list) {
                pos.x -= v;
            }
            while (!pipe_list.empty()) {
                if (pipe_list[0].x < -WIDTH) {
                    pipe_list.pop_front();
                    score += 1;
                    if (score % 10 == 0) {
                        v += 0.5;
                    }
                } else {
                    break;
                }
            }
            static double space = Bird::SIZE * 1.5;
            if (pipe_list.back().x < GAME_WIDTH - WIDTH - space) {
                new_pillar();
                space = ((double)dist(engine) / MAX_HEIGHT) * 2 * Bird::SIZE;
            }
        }

        void draw() noexcept {
            for (auto& pos : pipe_list) {
                DrawTexture(
                    texture,
                    (int)std::round(pos.x),
                    (int)std::round(pos.y),
                    WHITE
                );
            }
        }

        void reset() noexcept {
            v = INIT_V;
            pipe_list.clear();
            new_pillar();
        }

        friend bool collision_detection(const Bird& bird, const Pillar& pillar) noexcept;
};


inline bool overlap1d(
    const std::pair<double, double>& range1,
    const std::pair<double, double>& range2
) noexcept {
    return !(range1.second - range2.first < 1. || range2.second - range1.first < 1.);
}


bool collision_detection(const Bird& bird, const Pillar& pillar) noexcept {
    const std::pair<double, double> bird_x_range = {Bird::POS_X, Bird::POS_X+Bird::SIZE};
    const std::pair<double, double> bird_y_range = {bird.y, bird.y+Bird::SIZE};
    for (auto& pipe : pillar.pipe_list) {
        if (
            overlap1d(
                bird_x_range,
                {pipe.x, pipe.x+Pillar::WIDTH}
            )
            && overlap1d(
                bird_y_range,
                {pipe.y, pipe.y+Pillar::MAX_HEIGHT}
            )
        ) {
            return true;
        }
    }
    return false;
}


void draw_center(const std::vector<const char*>& text_list, Font& font, double font_size, Color color) noexcept {
    static auto text_size = MeasureTextEx(font, "XXX", font_size, 0.);
    int height = (GAME_HEIGHT - text_size.y*text_list.size()) / 2;
    for (auto text : text_list) {
        int width = MeasureText(text, font_size);
        DrawText(text, (GAME_WIDTH - width) / 2, height, font_size, color);
        height += text_size.y;
    }
}


void draw_score(Font& font, double font_size) noexcept {
    static auto text_size = MeasureTextEx(font, "Score: xxxxxx", font_size, 0.);
    auto text = std::format("score: {}", score);
    DrawText(text.c_str(), GAME_WIDTH - text_size.x, 0, font_size, BLACK);
}


int main() {
    InitWindow(GAME_WIDTH, GAME_HEIGHT, "flappy bird");
    SetTargetFPS(GAME_FPS);
    SetExitKey(KEY_Q);
    auto font = LoadFont("resource/LiberationMono-Regular.ttf");

    auto bg_img = LoadImage("resource/background.png");
    ImageResize(&bg_img, GAME_WIDTH, GAME_HEIGHT);
    UnloadImage(bg_img);
    auto bg = LoadTextureFromImage(bg_img);

    Bird bird;
    Pillar pillar;
    while (!WindowShouldClose()) {
        switch (game_status) {
            case GameStatus::PENDING: {
                if (IsKeyPressed(KEY_SPACE)) {
                    game_status = GameStatus::RUNNING;
                }
                break;
            }
            case GameStatus::RUNNING: {
                if (IsKeyPressed(KEY_ESCAPE)) {
                    game_status = GameStatus::PAUSE;
                }
                if (IsKeyPressed(KEY_SPACE)) {
                    bird.jump();
                }
                bird.update();
                pillar.update();
                if (bird.hit_top() || bird.hit_bottom() || collision_detection(bird, pillar)) {
                    game_status = GameStatus::ENDING;
                }
                break;
            }
            case GameStatus::PAUSE: {
                if (IsKeyPressed(KEY_ESCAPE)) {
                    game_status = GameStatus::RUNNING;
                }
                break;
            }
            case GameStatus::ENDING: {
                if (IsKeyPressed(KEY_SPACE)) {
                    bird.reset();
                    pillar.reset();
                    score = 0;
                    game_status = GameStatus::RUNNING;
                }
                break;
            }
            default: {
                break;
            }
        }
        BeginDrawing();
            DrawTexture(bg, 0, 0, WHITE);
            pillar.draw();
            bird.draw();
            draw_score(font, 30);
            switch (game_status) {
                case GameStatus::PENDING: {
                    draw_center(
                        {
                            "<SPACE> -> JUMP",
                            "<ESC> -> PAUSE",
                            "<Q> -> QUIT",
                            "press <SPACE> to start"
                        },
                        font,
                        30,
                        RED
                    );
                    break;
                }
                case GameStatus::ENDING: {
                    draw_center(
                        {
                            "Game Over",
                            "press <SPACE> to continue",
                            "press <Q> to quit"
                        },
                        font,
                        30,
                        RED
                    );
                    break;
                }
                default: {
                    break;
                }
            }
        EndDrawing();
    }
    UnloadTexture(bg);
    CloseWindow();
}
