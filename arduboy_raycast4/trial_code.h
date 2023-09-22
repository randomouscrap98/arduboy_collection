
constexpr uint8_t FLOORSTART = MIDSCREENY + 1;
constexpr flot FLOORDIST[HEIGHT - FLOORSTART] = {
    //-32.0/32, -31.0/32, -30.0/32, -29.0/32, -28.0/32, -27.0/32, -26.0/32, -25.0/32,
    //-24.0/32, -23.0/32, -22.0/32, -21.0/32, -20.0/32, -19.0/32, -18.0/32, -17.0/32,
    //-16.0/32, -15.0/32, -14.0/32, -13.0/32, -12.0/32, -11.0/32, -10.0/32, -9.0/32,
    //-8.0/32,  -7.0/32,  -6.0/32,  -5.0/32,  -4.0/32,  -3.0/32,  -2.0/32,  -1.0/22
    32.0/1, 32.0/2, 32.0/3, 32.0/4, 32.0/5, 32.0/6, 32.0/7, 32.0/8,
    32.0/9, 32.0/10, 32.0/11, 32.0/12, 32.0/13, 32.0/14, 32.0/15, 32.0/16,
    32.0/17, 32.0/18, 32.0/19, 32.0/20, 32.0/21, 32.0/22, 32.0/23, 32.0/24,
    32.0/25, 32.0/26, 32.0/27, 32.0/28, 32.0/29, 32.0/30, 32.0/31, //32.0/32,
    //0, -1.0/32, -2.0/32, -3.0/32, -4.0/32, -5.0/32, -6.0/32, -7.0/32,
    //-8.0/32, -9.0/32, -10.0/32, -11.0/32, -12.0/32, -13.0/32, -14.0/32, -15.0/32,
    //-16.0/32, -17.0/32, -18.0/32, -19.0/32, -20.0/32, -21.0/32, -22.0/32, -23.0/32,
    //-24.0/32, -25.0/32, -26.0/32, -27.0/32, -28.0/32, -29.0/32, //-22.0/32, -23.0/32,
};


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