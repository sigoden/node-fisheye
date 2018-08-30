const fs = require("fs");
const path = require("path");
const fisheye = require("..");
const { promisify } = require("util");

async function loadImages(folder) {
  let files = await findImageFiles(folder);
  return Promise.all(files.map(readImage));
}

async function findImageFiles(folder) {
  let files = await promisify(fs.readdir)(folder);
  return files.map(file => path.join(folder, file));
}

async function readImage(imgFile) {
  return promisify(fs.readFile)(imgFile);
}

async function calibrate(folder, checkboardWidth, checkboardHeight) {
  let imgs = await loadImages(folder);
  return fisheye.calibrate(imgs, checkboardWidth, checkboardHeight);
}

module.exports = calibrate;