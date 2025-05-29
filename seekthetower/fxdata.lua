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
image_helper("cursorselectimg", image("resources/cursor_select.png"))
image_helper(
	"itemsheet",
	image({
		filename = "resources/items.png",
		width = 8,
		height = 8,
		spacing = 2,
	})
)

-- We have some file with item definitions in it
local masterlist = toml(file("items.toml"))
-- print(masterlist)
-- print(#masterlist)

local maxstacks = {}
local prices = {}
local names = {}
local names_ptr = 0
local names_offsets = {}
for _, item in ipairs(masterlist.items) do
	table.insert(maxstacks, item.maxstack)
	table.insert(prices, item.buy)
	local name = item.name .. "\0"
	table.insert(names, name)
	table.insert(names_offsets, names_ptr)
	names_ptr = names_ptr + #name
end

field("itemnames")
for i, name in ipairs(names) do
	-- print("Writing name: " .. name .. " | offset: " .. names_offsets[i])
	write(name)
end

field("itemnameoffsets")
write(bytes(names_offsets, "uint16"))

field("itemstacks")
write(bytes(maxstacks))

field("itemprices")
write(bytes(prices))

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
