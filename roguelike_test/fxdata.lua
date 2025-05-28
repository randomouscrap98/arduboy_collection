-- Tiles (use automatic mipmapping) --
tiles, frames, width, height = image({
	filename = "resources/tilesheet.png",
	width = 32,
	height = 32,
	rawtiles = true,
})
raycast_helper("tilesheet", false, {
	["32"] = tiles,
	["16"] = image_resize(tiles, width, height, 16, 16),
	["8"] = image_resize(tiles, width, height, 8, 8),
	["4"] = image_resize(tiles, width, height, 4, 4),
})

-- TODO: sprites will use manual mipmapping later
tiles, frames, width, height = image({
	filename = "resources/spritesheet.png",
	width = 32,
	height = 32,
	rawtiles = true,
})
-- NOTE: true here is use mask!!
raycast_helper("spritesheet", true, {
	["32"] = tiles,
	["16"] = image_resize(tiles, width, height, 16, 16),
	["8"] = image_resize(tiles, width, height, 8, 8),
	["4"] = image_resize(tiles, width, height, 4, 4),
})

image_helper("menu", image("resources/menu.png"))
image_helper("bg", image("resources/bg_96x56.png"))
image_helper("titleimg", image("resources/title.png"))
image_helper("blankimg", image("resources/blank.png"))
image_helper("cursorimg", image("resources/cursor.png"))
image_helper(
	"itemsheet",
	image({
		filename = "resources/items.png",
		width = 8,
		height = 8,
		spacing = 2,
	})
)

-- Sprites (use manual mipmapping) --
-- function loadss(swidth)
-- 	sprites = image({
-- 		filename = "spritesheet" .. swidth .. ".png",
-- 		width = swidth,
-- 		height = swidth,
-- 		usemask = true,
-- 		rawtiles = true,
-- 	})
-- 	return sprites
-- end
--
-- raycast_helper("spritesheet", true, {
-- 	["32"] = loadss(32),
-- 	["16"] = loadss(16),
-- 	["8"] = loadss(8),
-- 	["4"] = loadss(4),
-- })

begin_save()
