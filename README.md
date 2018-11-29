# hairRemoval

Esse algoritmo tem o objetivo de remover pelos de imagens de pele com lesões.

## Como rodar

* Precisa do OpenCV
    * "sudo pacman -S opencv" no arch linux ou derivados
    * "sudo apt-get install libopencv-dev" no debian ou derivados como o ubuntu.
    * Foi testado apenas no mac e nas duas distribuições linux citadas acima.
    * Compile o programa com "make" e rode com ./hair arg1 arg2 arg3 arg4 arg5


Como imagens diferentes tem resultados melhores para parametros diferentes, a execução do programa tem 5 argumentos:

1. largura maxima (use um valor grande 10-50)
2. Comprimento minimo (Só funcionou bem com valores 0-10)
3. Threshold de segmentação(20-50)
4. Quantos pixels de distancia pegar como candidatos da interpolação (use 1-30)
5. qual imagem (sem a extensão, ex: fig3). Use apenas png.

* Resultados parciais da execução, tais como o resultado final imgFinal.png encontram-se na pasta output.
* Na pasta docs encontram-se o slide da apresentação e o artigo

## Alguns exemplos de execução que deram bons resultados

* ./hair 50 2 24 30 fig4
* ./hair 50 2 24 30 fig3
* ./hair 30 1 10 20 fig6
* ./hair 10 2 10 10 fig7
