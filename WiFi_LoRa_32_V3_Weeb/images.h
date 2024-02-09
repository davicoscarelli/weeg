#ifndef IMAGES_H
#define IMAGES_H


const uint8_t logo_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00, 
  0x80, 0x8F, 0x0D, 0xC0, 0x00, 0x1E, 0x20, 0x00, 0x1E, 0x1C, 0x39, 0x1C, 
  0x9E, 0x23, 0x08, 0x1E, 0x01, 0x00, 0x0C, 0x80, 0x19, 0x88, 0x9C, 0x13, 
  0x88, 0x3C, 0x11, 0xC8, 0x1C, 0x10, 0x88, 0x00, 0x78, 0x00, 0xC0, 0x78, 
  0x38, 0xEF, 0x78, 0x38, 0x0F, 0x3C, 0x38, 0x06, 0x02, 0xF8, 0x01, 0x01, 
  0x00, 0xFB, 0x00, 0x00, 0x78, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 
  };

const uint8_t logo_width = 24;
const uint8_t logo_height = 24;


const uint8_t debugSymbol[] = {
  0x30, 0x00, 0xFE, 0x01, 0xFE, 0x01, 0xCE, 0x01, 0x87, 0x03, 0x87, 0x03, 
  0xCE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0x30, 0x00, };

const uint8_t debugSymbol_width = 10;
const uint8_t debugSymbol_height = 10; 

const uint8_t connectedSymbol[] = {
  0x00, 0x60, 0x00, 0xE0, 0x00, 0xE0, 0x00, 0xE6, 0x00, 0xE6, 0x00, 0xE6, 
  0x60, 0xE6, 0x60, 0xE6, 0x60, 0xE6, 0x60, 0xE6, 0x67, 0xE6, 0x67, 0xE6, 
  0x67, 0xE6, 0x67, 0xE6, 0x67, 0xE6, 0x66, 0x66, };


const uint8_t connectedSymbol_width = 16;
const uint8_t connectedSymbol_height = 13; 

const uint8_t disconnectedSymbol[] = {
  0x00, 0x00, 0x08, 0x70, 0x18, 0x50, 0x10, 0x50, 0x20, 0x57, 0x40, 0x55, 
  0xE0, 0x55, 0xE0, 0x55, 0xA0, 0x57, 0xAE, 0x57, 0xAA, 0x5D, 0xEA, 0x5F, 
  0x6E, 0x76, 0x00, 0x20, 0x00, 0x40, 0x00, 0x00, };

const uint8_t disconnectedSymbol_width = 16; 
const uint8_t disconnectedSymbol_height = 16; 

const uint8_t wifiDisconnectedSymbol[] = {
  0x00, 0x00, 0x00, 0x00, 0xE0, 0x07, 0x18, 0x18, 0xC6, 0x63, 0x39, 0x9C, 
  0x8F, 0xF1, 0x70, 0x0E, 0x08, 0x10, 0xF8, 0x1F, 0x10, 0x08, 0xC0, 0x03, 
  0xC0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, };

const uint8_t wifiDisconnectedSymbol_width = 16;
const uint8_t wifiDisconnectedSymbol_height = 16;

const uint8_t wifiConnectedSymbol[] = {
  0x00, 0x00, 0x00, 0x00, 0xF0, 0x0F, 0xFC, 0x3F, 0x1E, 0x78, 0xC7, 0xE3, 
  0xF0, 0x0F, 0x3C, 0x3C, 0x8C, 0x31, 0xE0, 0x07, 0x70, 0x0E, 0x00, 0x00, 
  0x80, 0x01, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, };

const uint8_t wifiConnectedSymbol_width = 16;
const uint8_t wifiConnectedSymbol_height = 16;

#endif
