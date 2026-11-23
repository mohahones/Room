#pragma once

#include "../TableCards/Cards.cpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <vector>

	class Animation
	{
	private:
		struct CardState
		{
			bool moving = false;
			bool played = false;
			int playedSlot = -1;
			float animationTime = 0.0f;
			glm::vec3 start{0.0f};
			glm::vec3 finish{0.0f};
		};
		std::array<CardState, 5> cards{};

		static constexpr float CardWidth = 0.76f;
		static constexpr float CardHeight = 1.08f;
		static constexpr std::array<int, 5> PlayOrder = {2, 1, 3, 0, 4};
		std::vector<int> deck = {0, 1, 2, 3, 4};
		std::mt19937 randomEngine{std::random_device{}()};
		int playedCount = 0;

	public:
		void update(float deltaTime)
		{
			for (CardState &card : cards)
				if (card.moving)
					card.animationTime += deltaTime;
		}

		void playRandomCard()
		{
			if (deck.empty())
				return;

			std::uniform_int_distribution<size_t> choose(0, deck.size() - 1);
			const size_t selected = choose(randomEngine);
			CardState &card = cards[static_cast<size_t>(deck[selected])];
			card.start = deckPosition(selected);
			const int playedSlot = PlayOrder[static_cast<size_t>(playedCount++)];
			card.finish = playedPosition(playedSlot);
			card.playedSlot = playedSlot;
			card.animationTime = 0.0f;
			card.moving = true;
			deck.erase(deck.begin() + static_cast<std::vector<int>::difference_type>(selected));
		}

		void render(Shader &shader, Cards &cardRenderer, float cameraBlend)
		{
			for (size_t i = 0; i < deck.size(); ++i)
			{
				const size_t cardIndex = static_cast<size_t>(deck[i]);
				cardRenderer.renderCard(
					shader, cardIndex, cardModel(deckPosition(i), cameraBlend, 0.0f));
			}

			for (size_t i = 0; i < cards.size(); ++i)
			{
				CardState &card = cards[i];
				if (!card.moving && !card.played)
					continue;

				float progress = 1.0f;
				float spin = 0.0f;
				glm::vec3 center = playedPosition(std::max(card.playedSlot, 0));
				if (card.moving)
				{
					progress = std::clamp(card.animationTime / 0.8f, 0.0f, 1.0f);
					const float eased = progress * progress * (3.0f - 2.0f * progress);
					center = glm::mix(card.start, card.finish, eased);
					center.y += std::sin(progress * Pi) * 0.7f;
					spin = progress * 2.0f * Pi;
					if (progress >= 1.0f)
					{
						card.moving = false;
						card.played = true;
					}
				}

				cardRenderer.renderCard(shader, i, cardModel(center, cameraBlend, spin));
			}
		}

		glm::vec3 deckPosition(size_t index) const
		{
			constexpr float spacing = 0.86f;
			const float firstCardX = -0.5f * static_cast<float>(deck.size() - 1) * spacing;
			return glm::vec3(firstCardX + static_cast<float>(index) * spacing, 0.055f,
							 1.82f + TableDepthOffset);
		}

		static glm::vec3 playedPosition(int slot)
		{
			return glm::vec3((slot - 2) * 0.9f, 0.065f, TableDepthOffset);
		}

		static glm::mat4 cardModel(const glm::vec3 &center, float upright, float spin)
		{
			glm::vec3 raisedCenter = center;
			raisedCenter.y += CardHeight * 0.5f * upright;
			glm::mat4 model = glm::translate(glm::mat4(1.0f), raisedCenter);
			model = glm::rotate(model, -Pi * 0.5f * (1.0f - upright),
								glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, spin, glm::vec3(0.0f, 1.0f, 0.0f));
			return glm::scale(model, glm::vec3(CardWidth, CardHeight, 1.0f));
		}
	};
