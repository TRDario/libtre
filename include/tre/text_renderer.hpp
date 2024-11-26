#include "atlas.hpp"
#include "renderer_2d.hpp"

namespace tre {
	enum class AlignHorizontal : std::uint8_t {
		LEFT,
		CENTER,
		RIGHT
	};

	enum class Align : std::uint8_t {
		TOP_LEFT,
		TOP_CENTER,
		TOP_RIGHT,
		CENTER_LEFT,
		CENTER,
		CENTER_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_CENTER,
		BOTTOM_RIGHT
	};

	struct TextOutline {
		int thickness;
		tr::RGBA8 color;
	};

	inline constexpr TextOutline NO_OUTLINE{0, {0, 0, 0, 0}};

	struct Textbox {
		glm::vec2 pos;
		glm::vec2 posAnchor;
		glm::vec2 size;
		tr::AngleF rotation;
		Align textAlign;
	};

	class DynamicTextRenderer {
	  public:
		void setDPI(unsigned int dpi) noexcept;

		void setDPI(unsigned int x, unsigned int y) noexcept;

		void addUnformatted(int priority, const char* text, tr::TTFont& font, int fontSize, tr::TTFont::Style style,
							tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox);

		void addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize, tr::RGBA8 textColor,
						  TextOutline outline, const Textbox& textbox);

		void addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize,
						  std::span<tr::RGBA8> textColors, TextOutline outline, const Textbox& textbox);

		void forwardUpToPriority(Renderer2D& renderer, int minPriority);

		void forward(Renderer2D& renderer);

	  private:
		DynAtlas2D _atlas;
		std::map<int, std::vector<Textbox>> _textboxes;
		glm::uvec2 _dpi;
	};
} // namespace tre