"""Local reference contact sheets; never publish supplied movie excerpts."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from PIL import Image, ImageDraw

p = argparse.ArgumentParser()
p.add_argument('--ffmpeg', required=True)
p.add_argument('--input', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
records = []
for source in sorted(a.input.glob('*.mp4')):
    folder = a.output / source.stem
    folder.mkdir(exist_ok=True)
    probe = subprocess.run([a.ffmpeg, '-hide_banner', '-i', str(source)], capture_output=True, text=True)
    (folder / 'media.txt').write_text(probe.stderr, encoding='utf-8')
    subprocess.run([a.ffmpeg, '-hide_banner', '-loglevel', 'error', '-y', '-i', str(source),
                    '-vf', 'fps=4,scale=640:-1', str(folder / '%04d.png')], check=True)
    frames = sorted(folder.glob('[0-9][0-9][0-9][0-9].png'))
    records.append({'id': source.stem, 'sha256': hashlib.sha256(source.read_bytes()).hexdigest(),
                    'sampleRate': 4, 'samples': len(frames)})
    for start in range(0, len(frames), 24):
        batch = frames[start:start+24]
        with Image.open(batch[0]) as first:
            h = round(first.height / first.width * 384)
        sheet = Image.new('RGB', (384*4, (h+24)*6), '#101318')
        draw = ImageDraw.Draw(sheet)
        for k, path in enumerate(batch):
            with Image.open(path) as frame:
                frame.thumbnail((384, h))
                x, y = (k % 4)*384, (k//4)*(h+24)
                sheet.paste(frame, (x, y))
                draw.text((x+8, y+h+4), f'{source.stem}  {(start+k)/4:.2f}s', fill='white')
        sheet.save(a.output / f'{source.stem}-sheet-{start//24+1}.jpg', quality=94)
(a.output / 'references.json').write_text(json.dumps(records, indent=2), encoding='utf-8')
print(json.dumps(records, indent=2))
