# Fisheye camera model

A opencv fisheye camera model bindings for Node.js.


## Install

- install opencv 3.x

For linux
```
git clone https://github.com/opencv/opencv
mkdir opencv/build
cd opencv/build
cmake ..
sudo make install
```

For mac
```
brew tap homebrew/science
brew install opencv@3
brew link --force opencv@3
```

- install npm package

```
npm install @sigodenjs/fisheye
```

## Usage

### Prepare checkboard

Download the [checkerboard pattern](https://github.com/sigoden/node-fisheye/blob/master/doc/checkboard.webp?raw=true) and print it on a paper (letter or A4 size). You also want to attach the paper to a hard, flat surface such as a piece of cardboard. The key here: straight lines need to be straight.

### Take sample photos

Hold the pattern in front of your camera and capture some images. You want to hold the pattern in different positions and angles. The key here: the patterns need to appear distorted in a different ways (so that OpenCV knows as much about your lens as possible). 

![demo](https://raw.githubusercontent.com/sigoden/node-fisheye/master/doc/sample.png) 

### Find K and D

```js
let imgs = fs
    .readdirSync('example/samples')
    .map(file => fs.readFileSync('example/samples/'+file));
let {K, D} = fisheye.calibrate(imgs, 9, 6);
```

### Undistort image

```js
let img = fs.readFileSync('example/samples/IMG-0.jpg');
let buf = fisheye.undistort(img, K, D);
fs.writeFileSync('/tmp/IMG-0.jpg', buf);
```

![before](https://raw.githubusercontent.com/sigoden/node-fisheye/master/example/samples/IMG-0.jpg) --> ![after](https://raw.githubusercontent.com/sigoden/node-fisheye/master/doc/IMG-0.jpg)

## License

Copyright (c) 2018 sigoden

Licensed under the MIT license.