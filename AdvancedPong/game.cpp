#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

// player_dp = derivative of position (velocity)
// player_ddp = derivative of the derivative of position (acceration)
float player_1_pos, player_1_dp, player_2_pos, player_2_dp;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, initial_ball_dp_x = 90, ball_dp_x = initial_ball_dp_x, 
ball_dp_y, ball_half_size = 1;
float ball_speed = 150;

int player_1_score, player_2_score;

internal void
simulate_player(float* p, float* dp, float ddp, float dt) {
	// Applying friction
	ddp -= *dp * 15.f;

	// Some calculus for movement
	*p = *p + *dp * dt + ddp * dt * dt * 0.5f;
	*dp = *dp + ddp * dt;

	// Collision for players hitting walls
	if (*p + player_half_size_y > arena_half_size_y) {	
		*p = arena_half_size_y - player_half_size_y;
		*dp *= -0.5f;
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp *= -0.5f;
	}
}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y) {
	return(p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y - hs1y < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button;
bool enemy_is_ai;

internal void
simulate_game(Input* input, float dt) {
	// Drawing the arena
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0x000000);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0x555555);

	if (current_gamemode == GM_GAMEPLAY) {
		float player_1_ddp = 0.f;
		if (!enemy_is_ai) {
			if (is_down(BUTTON_UP)) player_1_ddp += 2000;
			if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
		}
		else {
			if (ball_p_y > player_1_pos + 2.f) player_1_ddp += 1500;
			if (ball_p_y < player_1_pos - 2.f) player_1_ddp -= 1500;
		}

		float player_2_ddp = 0.f;
		if (is_down(BUTTON_W)) player_2_ddp += 2000;
		if (is_down(BUTTON_S)) player_2_ddp -= 2000;

		simulate_player(&player_1_pos, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_pos, &player_2_dp, player_2_ddp, dt);

		// Simulate Ball
		{
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			// Ball collision

			// if ball hits players
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size,
				80, player_1_pos, player_half_size_x, player_half_size_y)) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x = -ball_speed;
				ball_dp_y = (ball_p_y - player_1_pos) * 2 + player_1_dp * 0.75f;
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size,
				-80, player_2_pos, player_half_size_x, player_half_size_y)) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x = ball_speed;
				ball_dp_y = (ball_p_y - player_2_pos) * 2 + player_2_dp * 0.75f;
			}

			// If ball hits top or bottom
			else if (ball_p_y + ball_half_size > arena_half_size_y ||
				ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_dp_y *= -1;
			}

			// If ball hits left or right
			else if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_dp_x = -initial_ball_dp_x;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x = initial_ball_dp_x;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
			}
		}

		// Drawing numbers using rectangles
		draw_number(player_1_score, -10, 40, 1.f, 0xffffff);
		draw_number(player_2_score, 10, 40, 1.f, 0xffffff);

		// Drawing the ball
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

		// Drawing the players
		draw_rect(80, player_1_pos, player_half_size_x, player_half_size_y, 0xffffff);
		draw_rect(-80, player_2_pos, player_half_size_x, player_half_size_y, 0xffffff);
	}
	else {

		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}

		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xffffff);
			draw_text("MULTIPLAYER", 18, -10, 1, 0xaaaaaa);
		}
		else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
			draw_text("MULTIPLAYER", 18, -10, 1, 0xffffff);
		}

		draw_text("PONG", -80, 40, 2, 0xffffff);
		draw_text("BY HIRO SHIRAISHI", -80, 24, 0.75, 0xffffff);
		draw_text("NOVEMBER TWELVETH TWENTY TWENTY ONE", -80, 15, 0.75, 0xffffff);
	}
}