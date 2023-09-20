
void raycastFloor()
{
    clearRaycast();
    flot rayDirX0 = dirX - dirY;//- planeX;
    flot rayDirY0 = dirY + dirX;//- planeY;
    flot rayDirX1 = dirX + dirY;//+ planeX;
    flot rayDirY1 = dirY - dirX;//+ planeY;
    flot rayDirXDiff = rayDirX1 - rayDirX0;
    flot rayDirYDiff = rayDirY1 - rayDirY0;
    flot fpx = (flot)posX, fpy = (flot)posY;

    uint16_t texture[TILESIZE];
    for(uint8_t i = 0; i < TILESIZE; i++)
        texture[i] = readTextureStrip16(tilesheet, 1, i);

    for(uint8_t y = FLOORSTART; y < HEIGHT; y++)
    {
        //flot rowDistance = pgm_read_word(FLOORDIST + (y - FLOORSTART)); //FLOORDIST[y - FLOORSTART];
        flot rowDistance = FLOORDIST[y - FLOORSTART];
        flot floorStepX = rowDistance * rayDirXDiff * INVWIDTH;
        flot floorStepY = rowDistance * rayDirYDiff * INVWIDTH;

        flot floorX = fpx + rowDistance * rayDirX0;
        flot floorY = fpy + rowDistance * rayDirY0;

        for(int x = 0; x < VIEWWIDTH; ++x)
        {
            // the cell coord is simply got from the integer parts of floorX and floorY
            uint8_t cellX = floorX.getInteger();
            uint8_t cellY = floorY.getInteger();

            // get the texture coordinate from the fractional part
            uint8_t tx = (uint8_t)(TILESIZE * (floorX - cellX)) & (TILESIZE - 1);
            uint8_t ty = (uint8_t)(TILESIZE * (floorY - cellY)) & (TILESIZE - 1);

            floorX += floorStepX;
            floorY += floorStepY;

            arduboy.drawPixel(x,y, (texture[tx] & fastlshift16(ty)) ? WHITE : BLACK);
        }
    }
}