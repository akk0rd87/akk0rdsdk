
  *How to generate the test set*

  Start with an image `flower.png`.
 - flower_alpha.png:
   ```
   convert flower.png -set colorspace Gray -separate -average grey.png
   convert flower.png grey.png -compose copy-opacity -composite flower_alpha.png
   rm grey.png
   ```
 - flower_cropped.jpg:
   ```
   convert flower.png -gravity center -crop 1040x1040+0+0 +repage flower_cropped.pnm
   cjpeg -outfile flower_cropped.jpg -quality 85 flower_cropped.pnm
   rm flower_cropped.pnm
   ```
 - flower.png.ffmpeg.y4m
    ```
    ffmpeg -i flower.png -pix_fmt yuvj420p flower.png.ffmpeg.y4m
    ```

 - flower.png.im_q85_*.jpg:
   ```
   convert flower.png flower.pnm
   cjpeg -outfile flower.png.im_q85_420.jpg -sample 2x2 -quality 85 flower.pnm
   cjpeg -outfile flower.png.im_q85_420_progr.jpg -sample 2x2 -quality 85 -progressive flower.pnm
   cjpeg -outfile flower.png.im_q85_422.jpg -sample 2x1 -quality 85 flower.pnm
   cjpeg -outfile flower.png.im_q85_440.jpg -sample 1x2 -quality 85 flower.pnm
   cjpeg -outfile flower.png.im_q85_444.jpg -sample 1x1 -quality 85 flower.pnm
   cjpeg -outfile flower.png.im_q85_444_1x2.jpg -sample 1x2,1x2,1x2 -quality 85 flower.pnm
   cjpeg -outfile flower.png.im_q85_gray.jpg -quality 85 -grayscale flower.pnm
   cjpeg -outfile flower.png.im_q85_luma_subsample.jpg -sample 1x1,2x2,2x2 -quality 85 flower.pnm
   rm flower.pnm
   ```

