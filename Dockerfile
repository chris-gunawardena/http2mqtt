FROM node:boron

WORKDIR /usr/src/app

COPY package.json .

RUN npm install

COPY . .

EXPOSE 443 1883

CMD [ "npm", "start" ]

