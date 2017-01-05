package com.cavan;

import java.io.File;

import de.greenrobot.daogenerator.DaoGenerator;
import de.greenrobot.daogenerator.Entity;
import de.greenrobot.daogenerator.Schema;

public class GreenDaoGenerator {

    private File mOutDir;
    private Schema mSchema;

    public GreenDaoGenerator(String outDir) {
        mOutDir = new File(outDir);
        mSchema = new Schema(1, "com.cavan.greendao");
        mSchema.enableKeepSectionsByDefault();
    }

    private void addEntities() {
        Entity entity = mSchema.addEntity("UserInfo");
        entity.addIdProperty().autoincrement();
        entity.addStringProperty("name").index();
        entity.addBooleanProperty("sex");
        entity.addIntProperty("age");

        entity = mSchema.addEntity("GroupInfo");
        entity.addIdProperty().autoincrement();
        entity.addStringProperty("name").index();
        entity.addStringProperty("description");
    }

    public boolean generateAll() {
        if (!mOutDir.isDirectory()) {
            if (!mOutDir.mkdirs()) {
                return false;
            }
        }

        addEntities();

        try {
            DaoGenerator generator = new DaoGenerator();
            generator.generateAll(mSchema, mOutDir.getPath());
            return true;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return false;
    }

    public static void main(String[] args) {
        GreenDaoGenerator generator = new GreenDaoGenerator(args[0]);
        generator.generateAll();
    }
}
