import { build, defineConfig } from 'vite';
import tailwindcss from '@tailwindcss/vite';
import htmlMinify from 'vite-plugin-html-minify';
import { viteSingleFile } from 'vite-plugin-singlefile';
import path from 'path';
import fs from 'fs';

for (const file of fs.readdirSync('./in').filter(file => file.endsWith('.html'))) {
    const name = file.replace('.html', '');

    await build(defineConfig({
        build: {
            outDir: path.resolve('./dist'),
            emptyOutDir: false,
            rollupOptions: {
                input: path.join('./in', file),
                output: {
                    entryFileNames: `${name}.html`,
                    assetFileNames: `${name}.[ext]`,
                },
            },
        },
        plugins: [
            tailwindcss(),
            viteSingleFile({
                removeViteModuleLoader: true,
            }),
            htmlMinify(),
        ],
    }));
}
